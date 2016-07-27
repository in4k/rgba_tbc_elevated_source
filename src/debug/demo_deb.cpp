//
//                              Elevated
//
// Winner 4 kb intro of Breakpoint 2009, by TBC and RGBA 
//
//  . Music by Puryx (Christian Rønde)
//  . Visuals by iq (Inigo Quilez)
//  . Sound synthesizer and code optimization by Mentor (Rune L. H. Stubbe)
//  . Crinkler compressor (http://www.crinkler.net)
//
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
//
// Visual Studio 2015, Nasm, Crinkler

#include "..\d3d9\d3dx9.h"

#include <stdio.h>
#include "..\constants.h"
#include "..\synth.h"
#include "..\comcall.h"
#include "sync.h"

#pragma intrinsic(sin)
#pragma intrinsic(cos)
#pragma intrinsic(exp)

extern "C" 
{
    extern float constantPool[256];
	extern char ubershader[];
    extern int ubershaderLen;
	extern float fullscreenQuad[];
	extern float fullscreenQuad2[];

    extern void WINAPI TextureFillCallback(D3DXVECTOR4* pOut, CONST D3DXVECTOR2* pTexCoord,  CONST D3DXVECTOR2* pTexelSize,  LPVOID pData);

};


LPDIRECT3DVERTEXSHADER9 compileVertexShader(const char* entrypoint) 
{
	LPD3DXBUFFER shader;
	LPD3DXBUFFER errors;
	LPD3DXCONSTANTTABLE ctable;

	if(D3DXCompileShader(ubershader, ubershaderLen, NULL, NULL, entrypoint, "vs_3_0", 0, &shader, &errors, &ctable) != D3D_OK) 
    {
		MessageBox(0, (const char*)errors->GetBufferPointer(), 0, 0);
		return nullptr;
	}

	printf("shader: %s\n", entrypoint);
	for(int i = 0; ;i++) 
    {
		D3DXHANDLE h = ctable->GetConstant(NULL, i);
		if(h == NULL)
			break;

		D3DXCONSTANT_DESC desc;
		UINT s = 1;
		ctable->GetConstantDesc(h, &desc, &s);
		printf("%s: %d-%d\n", desc.Name, desc.RegisterIndex, desc.RegisterIndex+desc.RegisterCount);

	} 
	printf("\n");

	LPDIRECT3DVERTEXSHADER9 vertexshader;
	COMHandles.device->CreateVertexShader((DWORD*)shader->GetBufferPointer(), &vertexshader);
	return vertexshader;
}

static LPDIRECT3DPIXELSHADER9 compilePixelShader(const char* entrypoint) 
{
	LPD3DXBUFFER shader;
	LPD3DXBUFFER errors;
	LPD3DXCONSTANTTABLE ctable;
	if(D3DXCompileShader(ubershader, ubershaderLen, NULL, NULL, entrypoint, "ps_3_0", 0, &shader, &errors, &ctable) != D3D_OK) 
    {
		MessageBox(0, (const char*)errors->GetBufferPointer(), 0, 0);
		return nullptr;
	}
	
	printf("shader: %s\n", entrypoint);
	for(int i = 0; ;i++) 
    {
		D3DXHANDLE h = ctable->GetConstant(NULL, i);
		if(h == NULL)
			break;

		D3DXCONSTANT_DESC desc;
		UINT s = 1;
		ctable->GetConstantDesc(h, &desc, &s);
		printf("%s: %d-%d\n", desc.Name, desc.RegisterIndex, desc.RegisterIndex+desc.RegisterCount);

	} 
	printf("\n");
	
	LPDIRECT3DPIXELSHADER9 pixelshader;
	COMHandles.device->CreatePixelShader((DWORD*)shader->GetBufferPointer(), &pixelshader);
	return pixelshader;
}

bool DemoInit( void ) 
{
	COMHandles.vertexShader0 = compileVertexShader("m0");
	COMHandles.pixelShader0 = compilePixelShader("m1");
	COMHandles.pixelShader1 = compilePixelShader("m2");
	COMHandles.pixelShader2 = compilePixelShader("m3");
	COMHandles.pixelShader3 = compilePixelShader("m4");

    if( !COMHandles.vertexShader0 || !COMHandles.pixelShader0 ||
	    !COMHandles.pixelShader1  || !COMHandles.pixelShader2 || 
        !COMHandles.pixelShader3 )
        return false;

	COMHandles.device->CreateTexture(WIDTH, HEIGHT, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT, &COMHandles.renderTarget0.tex, NULL);
	COMHandles.renderTarget0.tex->GetSurfaceLevel(0, &COMHandles.renderTarget0.surf);
	COMHandles.device->CreateTexture(WIDTH, HEIGHT, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &COMHandles.renderTarget1.tex, NULL);
	COMHandles.renderTarget1.tex->GetSurfaceLevel(0, &COMHandles.renderTarget1.surf);
	COMHandles.device->CreateRenderTarget(2,1,D3DFMT_A32B32G32R32F, D3DMULTISAMPLE_NONE, 0, true, &COMHandles.camRenderTarget, NULL);
	COMHandles.device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &COMHandles.backbuffer);

	//create landscape mesh	
	D3DXCreatePolygon(COMHandles.device, 52.0f, 4, &COMHandles.mesh, NULL);
    COMHandles.mesh->CloneMeshFVF(D3DXMESH_32BIT, D3DFVF_XYZ|D3DFVF_NORMAL, COMHandles.device, &COMHandles.mesh);
	D3DXTessellateNPatches(COMHandles.mesh, NULL, 512, false, &COMHandles.mesh, NULL);
    //D3DXTessellateNPatches(COMHandles.mesh, NULL, 1024, false, &COMHandles.mesh, NULL);
	COMHandles.mesh->OptimizeInplace(D3DXMESHOPT_VERTEXCACHE+D3DXMESHOPT_STRIPREORDER,0,0,0,0);


	LPDIRECT3DTEXTURE9 texture;
	COMHandles.device->CreateTexture(TRES, TRES, 1, 0, D3DFMT_R16F, D3DPOOL_MANAGED, &texture, NULL);
	D3DXFillTexture(texture, TextureFillCallback, 0);	//use synth random function - change if it doesn't suffice
	COMHandles.device->SetTexture(0, texture);
	COMHandles.device->SetTexture(D3DVERTEXTEXTURESAMPLER0, texture);

/*	
	for( int i=0; i<3; i++ )
	{
		COMHandles.device->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		COMHandles.device->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		COMHandles.device->SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
	}
*/
	COMHandles.device->SetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	COMHandles.device->SetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

    return true;
}


static void constructMatrix(D3DXMATRIX* mat)
{
	//cam stuff
	COMHandles.device->SetPixelShader(COMHandles.pixelShader0);
	COMHandles.device->SetRenderTarget(0, COMHandles.camRenderTarget);
	COMHandles.device->SetPixelShaderConstantF(0, constantPool, 64);
	COMHandles.device->SetFVF(D3DFVF_XYZRHW|D3DFVF_TEX1);
	COMHandles.device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, fullscreenQuad, 6*sizeof(float));

	D3DLOCKED_RECT lr;
	COMHandles.camRenderTarget->LockRect(&lr, NULL, 0);
	D3DXVECTOR4* pptr = (D3DXVECTOR4*)lr.pBits;
	*(D3DXVECTOR4*)&constantPool[PARAM_CAM] = pptr[0];

    const float up[3] = { sinf( pptr->w ), cosf( pptr->w ), 0.0f };

	D3DXMatrixLookAtLH(mat, (D3DXVECTOR3*)&pptr[0], (D3DXVECTOR3*)&pptr[1], (D3DXVECTOR3*)up);
	D3DXMATRIX tmp;
	D3DXMatrixPerspectiveFovLH(&tmp, constantPool[PARAM_CAM_FOV], WIDTH/(float)HEIGHT, 3.125e-002f, 256.0f);
	
	D3DXMatrixMultiply(mat, mat, &tmp);
	COMHandles.camRenderTarget->UnlockRect();
}

void DemoEffect(int dummy, int position) 
{
	COMHandles.device->BeginScene();
position = SAMPLE_RATE*70;
	const float ftime = position / (float)SAMPLE_RATE;


	
	int d = position;
	for(int i = 0; i < 8; i++) 
    {
		constantPool[PARAM_INSTRUMENT_SYNC+i*4] = float(d);
	}
	
	int r = 0;
	do {
		int p = sequence_data[NUM_ROWS*2 + (r>>4)];
		p = pattern_data[(p<<4)|(r&0xF)];
		if(p) 
        {
			constantPool[PARAM_INSTRUMENT_SYNC+(p&7)*4] = float(d);
		}
		r++;
		d -= MAX_NOTE_SAMPLES;
	} while(d>=0);


	//---------------------------
	// animation
	//---------------------------
	constantPool[PARAM_CAM_SEEDX] = SyncParam(position,"camSeedX")/256.0f;						//  0..255
	constantPool[PARAM_CAM_SEEDY] = SyncParam(position,"camSeedY")/256.0f;						//  0..255
	constantPool[PARAM_CAM_SPEED] = SyncParam(position,"camSpeed")/4096.0f;						//  0..0.125  precission 0.0004882
	constantPool[PARAM_CAM_FOV]   = SyncParam(position,"camFov")/96.0f;							//  0..114°  precission 0.447°

	constantPool[PARAM_CAM_POSY] = (SyncParam(position,"camPosY")/64.0f);							//  0..4     precission 0.0015625
	constantPool[PARAM_CAM_TARY] = (SyncParam(position,"camTarY")-128.0f)/4.0f;	//-32..32    precission 0.125


	constantPool[PARAM_SUN_ANGLE] = SyncParam(position,"sun_angle")/32.0f;						//  0..458°  precision is 1.79°
	constantPool[PARAM_WATER_LEVEL] = (SyncParam(position,"terWaterLevel")-192.0f)/128.0f;		// -2..2     precission 0.015625
	constantPool[PARAM_SEASON] = SyncParam(position,"terSeason")/256.0f;							//  0..1     precission 0.003906
	constantPool[PARAM_BRIGHTNESS] = (SyncParam(position,"imgBrightness")-128.0f)/128.0f;		// -1..1	 precission 0.07...
	constantPool[PARAM_CONTRAST]   = SyncParam(position,"imgContrast")/128.0f;			        // 0..2	     precission 0.07...
	constantPool[PARAM_TERRAIN_SCALE] = (SyncParam(position,"terScale")-128.0f)/128.0f;			// -1..1	 precission 0.015625

	constantPool[PARAM_SUNDIR_X] = cosf(constantPool[PARAM_SUN_ANGLE]);	//sundir.x
	constantPool[PARAM_SUNDIR_Y] = 0.3125f;								//sundir.y
	constantPool[PARAM_SUNDIR_Z] = sinf(constantPool[PARAM_SUN_ANGLE]);	//sundir.z

	COMHandles.device->SetVertexShader(COMHandles.vertexShader0);

	constantPool[PARAM_TIME] = ftime;
	constructMatrix((D3DXMATRIX*)&constantPool[64]);

	// zbuffer
	COMHandles.device->SetVertexShaderConstantF(0, constantPool, 64);
	COMHandles.device->SetPixelShaderConstantF(0, constantPool, 64);
	COMHandles.device->SetPixelShader(COMHandles.pixelShader1);

  	COMHandles.device->SetRenderTarget(0, COMHandles.renderTarget0.surf);
	COMHandles.device->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
	COMHandles.mesh->DrawSubset(0);

	D3DXMatrixInverse((D3DXMATRIX*)&constantPool[64], NULL, (D3DXMATRIX*)&constantPool[64]);
	COMHandles.device->SetPixelShaderConstantF(0, constantPool, 64);
				
	// colorize
	COMHandles.device->SetRenderTarget(0, COMHandles.renderTarget1.surf);
	//COMHandles.device->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
	COMHandles.device->SetFVF(D3DFVF_XYZRHW|D3DFVF_TEX1);
	COMHandles.device->SetPixelShader(COMHandles.pixelShader2);
	COMHandles.device->SetTexture(1, COMHandles.renderTarget0.tex);
	COMHandles.device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, fullscreenQuad, 6*sizeof(float));


	constantPool[PARAM_TIME] += (1.0f/(float)FILMRATE);
	constructMatrix((D3DXMATRIX*)&constantPool[64]);
	COMHandles.device->SetPixelShaderConstantF(0, constantPool, 64);
  
	// mblur
	COMHandles.device->SetRenderTarget(0, COMHandles.backbuffer);

	if( LETTERBOX )
	COMHandles.device->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0, 1.0f, 0);

	//COMHandles.device->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
	COMHandles.device->SetPixelShader(COMHandles.pixelShader3);
	COMHandles.device->SetTexture(2, COMHandles.renderTarget1.tex);
	if( LETTERBOX )
	COMHandles.device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, fullscreenQuad2, 6*sizeof(float));
	else
	COMHandles.device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, fullscreenQuad, 6*sizeof(float));
	  
	COMHandles.device->EndScene();
	COMHandles.device->Present(0, 0, 0, 0);	
}