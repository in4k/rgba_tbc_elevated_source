;
;                              Elevated
;
; Winner 4 kb intro of Breakpoint 2009, by TBC and RGBA 
;
;  . Music by Puryx (Christian Rønde)
;  . Visuals by iq (Inigo Quilez)
;  . Sound synthesizer and code optimization by Mentor (Rune L. H. Stubbe)
;  . Crinkler compressor (http://www.crinkler.net)
;
; License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
;
; Visual Studio 2015, Nasm, Crinkler


%include "src\music.h"
%include "src\comcall.nh"
%include "src\constants.h"

global _sync_timeline
global _sync_timeline_end
global _update_sync@4
global _shader_entrypoint
global _demoinit_asm@0
global _demoeffect_asm


extern __imp__MessageBoxA@16
extern _sequence_data
extern _pattern_data
extern _constantPool
extern _frandom@0
;extern _notes
extern __imp__D3DXCompileShader@40
extern __imp__D3DXCreatePolygon@20
extern __imp__D3DXTessellateNPatches@24
extern __imp__D3DXMatrixLookAtLH@16
extern __imp__D3DXMatrixPerspectiveFovLH@20
extern __imp__D3DXMatrixMultiply@12
extern __imp__D3DXMatrixInverse@12
extern __imp__D3DXFillTexture@12
extern _fullscreenQuad
extern _fullscreenQuad2
extern _TextureFillCallback@16
extern _sample_rate
extern _ubershader
extern _ubershaderLen

%define D3DPOOL_DEFAULT				0
%define D3DFMT_A8R8G8B8				21
%define D3DFMT_A16B16G16R16F		113
%define D3DFMT_A32B32G32R32F		116
%define D3DUSAGE_RENDERTARGET		1
%define D3DBACKBUFFER_TYPE_MONO		0
%define D3DFMT_R16F					111
%define D3DFMT_R32F					114
%define D3DPOOL_MANAGED				1
%define D3DVERTEXTEXTURESAMPLER0	257
%define D3DSAMP_ADDRESSU			1
%define D3DSAMP_ADDRESSV			2
%define D3DTADDRESS_CLAMP			3
%define D3DFVF_XYZ					0x02
%define D3DFVF_NORMAL				0x10
%define D3DXMESH_32BIT				1
%define D3DPT_TRIANGLESTRIP			5
%define D3DCLEAR_TARGET				1
%define D3DCLEAR_ZBUFFER			2
%define D3DFVF_XYZRHW				4
%define D3DFVF_TEX1					0x100

section strvsps data align=1
_str_vs: db "vs_3_0", 0
_str_ps: db "ps_3_0", 0

section filmra data align=4
_invfilmrate:	dd 0x3D2B0000	;~1/24

section tmpmem bss align=4
_tmpmem:	resd 4*4

section shadent data align=1
_shader_entrypoint:
db "m0", 0

section parscl data align=4
_param_scales:
dw	0,	256			;camSeedX
dw	0,	256			;camSeedY
dw	0,	4096		;camSpeed
dw	0,	96			;camFov
dw	0,	64			;camPosY
dw	128,4			;camTarY
dw	0,	32			;sun_angle
dw	192,128			;terWaterLevel
dw	0,	256			;terSeason
dw	128,128			;imgBrightness
dw	0,	128			;imgContrast
dw	128,128			;terScale

section sync data align=1
_sync_timeline:
incbin "src\release\timeline.sync"
_sync_timeline_end:

section	demoinit code align=1
_demoinit_asm@0:
pusha
	mov esi, comhandle(dummy0)

	; D3DXCompileShader(myshader, strlen(myshader), NULL, NULL, shader_entrypoint, "vs_3_0", 0, &shader, NULL, NULL);
	push byte 0
	push byte 0
	push esi
	push byte 0
	push _str_vs
	push _shader_entrypoint
	push byte 0
	push byte 0
	push dword [_ubershaderLen]
	push _ubershader
	call [__imp__D3DXCompileShader@40]
	lodsd
	
	;COMHandles.device->CreateVertexShader((DWORD*)shader->GetBufferPointer(), (IDirect3DVertexShader9**)sptr);
	push esi
	comcall eax, GetBufferPointer

	push eax
	comcall [comhandle(device)], CreateVertexShader
	lodsd

	mov bl, 4
.pixelshader_loop:
	inc byte [_shader_entrypoint+1]
	

	; D3DXCompileShader(myshader, strlen(myshader), NULL, NULL, shader_entrypoint, "ps_3_0", 0, &shader, NULL, NULL);
	push byte 0
	push eax
	push esi
	push byte 0
	push _str_ps
	push _shader_entrypoint
	push byte 0
	push byte 0
	push dword [_ubershaderLen]
	push _ubershader
	call [__imp__D3DXCompileShader@40]
	lodsd
	
	;COMHandles.device->CreatePixelShader((DWORD*)shader->GetBufferPointer(), (IDirect3DPixelShader9**)sptr);
	push esi
	comcall eax, GetBufferPointer
	push eax
	comcall [comhandle(device)], CreatePixelShader
	lodsd
	
	dec bl
	jnz .pixelshader_loop
	
	; COMHandles.device->CreateTexture(WIDTH, HEIGHT, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT, &COMHandles.renderTarget0.tex, NULL);
	push byte 0
	push esi
	push byte D3DPOOL_DEFAULT
	push byte D3DFMT_A32B32G32R32F
	push byte D3DUSAGE_RENDERTARGET
	push byte 1
	push HEIGHT
	push WIDTH
	comcall [comhandle(device)], CreateTexture
	
	; COMHandles.renderTarget0.tex->GetSurfaceLevel(0, &COMHandles.renderTarget0.surf);
	lodsd
	push esi
	push byte 0
	comcall eax, GetSurfaceLevel
	lodsd
	
	; COMHandles.device->CreateTexture(WIDTH, HEIGHT, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &COMHandles.renderTarget0.tex, NULL);
	push byte 0
	push esi
	push byte D3DPOOL_DEFAULT
	push byte D3DFMT_A8R8G8B8
	push byte D3DUSAGE_RENDERTARGET
	push byte 1
	push HEIGHT
	push WIDTH
	comcall [comhandle(device)], CreateTexture
	
	; COMHandles.renderTarget0.tex->GetSurfaceLevel(0, &COMHandles.renderTarget1.surf);
	lodsd
	push esi
	push byte 0
	comcall eax, GetSurfaceLevel
	lodsd
	
	; COMHandles.device->CreateRenderTarget(2,1,D3DFMT_A32B32G32R32F, D3DMULTISAMPLE_NONE, 0, true, &COMHandles.camRenderTarget, NULL);
	push byte 0
	push esi
	push byte 1
	push byte 0
	push byte 0
	push byte D3DFMT_A32B32G32R32F
	push byte 1
	push byte 2
	comcall [comhandle(device)], CreateRenderTarget
	lodsd
	
	; COMHandles.device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &COMHandles.backbuffer);
	push esi
	push byte D3DBACKBUFFER_TYPE_MONO
	push byte 0
	push byte 0
	comcall [comhandle(device)], GetBackBuffer
	lodsd
	
	;; create landscape mesh
	; D3DXCreatePolygon(COMHandles.device, 52.0f, 4, &COMHandles.mesh, NULL);
	push byte 0
	push esi
	push byte 4
	push 0x42500000	;52.0f
	push dword [comhandle(device)]
	call [__imp__D3DXCreatePolygon@20]
	
	; COMHandles.mesh->CloneMeshFVF(D3DXMESH_32BIT, D3DFVF_XYZ|D3DFVF_NORMAL, COMHandles.device, &COMHandles.mesh);
	push esi
	push dword [comhandle(device)]
	push byte D3DFVF_XYZ|D3DFVF_NORMAL
	push byte D3DXMESH_32BIT
	mov eax, dword [esi]
	comcall eax, CloneMeshFVF
	
	; D3DXTessellateNPatches(COMHandles.mesh, NULL, 512.0f, false, &COMHandles.mesh, NULL);
	push byte 0
	push esi
	push byte 0
	push 0x44000000	;512.0f
	push byte 0
	push dword [esi]
	call [__imp__D3DXTessellateNPatches@24]

	; COMHandles.mesh->OptimizeInplace(D3DXMESHOPT_VERTEXCACHE+D3DXMESHOPT_STRIPREORDER,0,0,0,0);
	push byte 0
	push byte 0
	push byte 0
	push byte 0
	push 0x04000000+0x08000000 ; D3DXMESHOPT_VERTEXCACHE+D3DXMESHOPT_STRIPREORDER
	mov eax, dword [esi]
	comcall eax, OptimizeInplace
	

	;; noise texture!
	; COMHandles.device->CreateTexture(TRES, TRES, 1, 0, D3DFMT_R16F, D3DPOOL_MANAGED, &texture, NULL);
	push eax
	mov eax, esp
	
	push byte 0
	push eax
	push byte D3DPOOL_MANAGED
	push byte D3DFMT_R16F
	push byte 0
	push byte 1
	push TRES
	push TRES
	comcall [comhandle(device)], CreateTexture
	pop ebp
	
	push byte 0
	push _TextureFillCallback@16
	push ebp
	call dword [__imp__D3DXFillTexture@12]
	
	; COMHandles.device->SetTexture(0, texture);
	push ebp
	push byte 0
	comcall [comhandle(device)], SetTexture
	
	; COMHandles.device->SetTexture(D3DVERTEXTEXTURESAMPLER0, texture);
	push ebp
	push D3DVERTEXTEXTURESAMPLER0
	comcall [comhandle(device)], SetTexture
	

	; COMHandles.device->SetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	push byte D3DTADDRESS_CLAMP
	push byte D3DSAMP_ADDRESSU
	push byte 2
	comcall [comhandle(device)], SetSamplerState
	
	; COMHandles.device->SetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);	
	push byte D3DTADDRESS_CLAMP
	push byte D3DSAMP_ADDRESSV
	push byte 2
	comcall [comhandle(device)], SetSamplerState	
	
popa
ret

;write cam at edi and increment edi by 4 dwords
section makematr code align=1
_create_matrix:
	; COMHandles.device->SetRenderTarget(0, COMHandles.camRenderTarget);
	push dword [comhandle(camRenderTarget)]
	push byte 0
	comcall [comhandle(device)], SetRenderTarget
	
	; COMHandles.device->SetPixelShaderConstantF(0, constantPool, 32);
	push byte NUM_CONSTANTS
	push _constantPool
	push byte 0
	comcall [comhandle(device)], SetPixelShaderConstantF
	
	; COMHandles.device->SetPixelShader(COMHandles.pixelShader0);
	push dword [comhandle(pixelShader0)]
	comcall [comhandle(device)], SetPixelShader
	
	; COMHandles.device->SetFVF(D3DFVF_XYZRHW|D3DFVF_TEX1);
	push D3DFVF_XYZRHW|D3DFVF_TEX1
	comcall [comhandle(device)], SetFVF
	
	; COMHandles.device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, fullscreenQuad, 6*sizeof(float));
	push byte 6*4
	push _fullscreenQuad
	push byte 2
	push byte D3DPT_TRIANGLESTRIP
	comcall [comhandle(device)], DrawPrimitiveUP

	; COMHandles.camRenderTarget->LockRect(&lr, NULL, 0);	
	push eax
	push eax
	mov eax, esp
	push byte 0
	push byte 0
	push eax
	comcall [comhandle(camRenderTarget)], LockRect_Surface
	pop eax
	pop esi
	
	;copy cam position
	lodsd
	stosd
	lodsd
	stosd
	lodsd
	stosd
	fld dword [esi]
	lodsd
	stosd
	
	
	fsincos
	fstp dword [_tmpmem+4]
	fstp dword [_tmpmem]

	; D3DXMatrixLookAtLH(mat, (D3DXVECTOR3*)&pptr[0], (D3DXVECTOR3*)&pptr[1], (D3DXVECTOR3*)up);
	push _tmpmem
	push esi
	sub esi, byte 4*4
	push esi
	push _constantPool+16*4*4
	call [__imp__D3DXMatrixLookAtLH@16]
	
	; D3DXMatrixPerspectiveFovLH(&tmp, constantPool[PARAM_CAM_FOV], WIDTH / (float)HEIGHT, 3.125e-002f, 256.0f);
	push 0x43800000		;256.0f
	push 0x3D000000		;3.125e-002f
	push 0x3FE00000		;aspect: 1.75
	push dword [_constantPool+PARAM_CAM_FOV*4]
	push _tmpmem
	call [__imp__D3DXMatrixPerspectiveFovLH@20]

	push _tmpmem
	push _constantPool+16*4*4
	push _constantPool+16*4*4
	call [__imp__D3DXMatrixMultiply@12]
	
	; COMHandles.camRenderTarget->UnlockRect();
	comcall [comhandle(camRenderTarget)], UnlockRect_Surface
ret

section	demoeff code align=1
_demoeffect_asm:
	pusha
	
	; COMHandles.device->BeginScene();
	comcall [comhandle(device)], BeginScene
	
	;; sync!!
	mov	esi, _sync_timeline
	mov ebx, dword [esp+10*4]	;ebx: position

	.timeline:
		mov edi, _constantPool	;constant pool
		mov ebp, _param_scales
		
		xor eax, eax
		lodsb
		imul eax, MAX_NOTE_SAMPLES*4
		push ebx
		push eax
		
		lodsw
		xchg edx, eax
		
		push byte 12
		pop ecx
		.unpack_loop:
			xor eax, eax
			lodsb
			push eax
			fild dword [esp]			;st0: val0
			pop eax
			
			add dx, dx
			jnc .dont_interp
			
			movzx eax, byte [esi+NUM_ROCKET_PARAMS+2]
			push eax
			fild dword [esp]			;st0: val1, st1: val0
			pop eax
			fsub st1					;st0: val1-val0, st1: val0
			fidiv dword [esp]			;st0: (val1-val0)/(rowsamples), st1: val0
			fimul dword [esp+4]			;st0: ((val1-val0)*sampletime)/(rowsamples), st1: val0
			faddp st1
		.dont_interp:
			fisub word [ebp]
			inc ebp
			inc ebp
			fidiv word [ebp]
			inc ebp
			inc ebp
			fstp dword [edi]
			scasd
			loop .unpack_loop
		pop eax
		sub ebx, eax
		pop eax
		jge .timeline
	.end:
	
	;set sun direction!
	fld dword [_constantPool+PARAM_SUN_ANGLE*4]
	fsincos
	fstp dword [edi]
	scasd
	mov dword [edi], 0x3ea00000	;0.3125
	scasd
	fstp dword [edi]
	scasd
	
	; constantPool[PARAM_TIME] = ftime
	fild dword [esp+10*4]
	fidiv dword [_sample_rate]
	fstp dword [edi]
	scasd
	
	;cam :/
	
	; constructMatrix((D3DXMATRIX*)&constantPool[16*4]);
	call _create_matrix
	
	; instrument sync data
	;xor edx, edx
	mov edx, NUM_ROWS*2*16
	mov ebx, dword [esp+10*4]	;ebx: position
	
	push byte 8*4
	pop ecx
	.clearloop:
		push ebx
		fild dword [esp]
		fstp dword [edi+ecx*4-4]
		pop eax
		loop .clearloop
		
	.beamloop:
		push edx
		mov eax, edx
		shr eax, byte 4
		movzx eax, byte [_sequence_data+eax]
		shl eax, byte 4
		
		and edx, byte 0xF
		movzx eax, byte [_pattern_data+eax+edx]
		test eax, eax
		jz .skipnote
		
		and eax, byte 7
		shl eax, byte 4
		
		push ebx
		fild dword [esp]
		fstp dword [edi+eax]
		pop eax
.skipnote:
		pop edx
		inc edx
		
		sub ebx, MAX_NOTE_SAMPLES
		jge .beamloop

	;; z-pass
	
	; COMHandles.device->SetVertexShaderConstantF(0, constantPool, 32);
	push byte NUM_CONSTANTS
	push _constantPool
	push byte 0
	comcall [comhandle(device)], SetVertexShaderConstantF
	
	; COMHandles.device->SetVertexShader(COMHandles.vertexShader0);
	push dword [comhandle(vertexShader0)]
	comcall [comhandle(device)], SetVertexShader
	
		; COMHandles.device->SetRenderTarget(0, COMHandles.renderTarget0.surf);	
	push dword [comhandle(surf0)]
	push byte 0
	comcall [comhandle(device)], SetRenderTarget

	; COMHandles.device->SetPixelShaderConstantF(0, constantPool, 32);	
	push byte NUM_CONSTANTS
	push _constantPool
	push byte 0
	comcall [comhandle(device)], SetPixelShaderConstantF
	
	; COMHandles.device->SetPixelShader(COMHandles.pixelShader1);
	push dword [comhandle(pixelShader1)]
	comcall [comhandle(device)], SetPixelShader

	; COMHandles.device->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0, 1.0f, 0);	
	push byte 0
	push 0x3f800000
	push byte 0
	push byte D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER
	push byte 0
	push byte 0
	comcall [comhandle(device)], Clear

	;; render landscape
	; COMHandles.mesh->DrawSubset(0);
	push byte 0
	comcall [comhandle(mesh)], DrawSubset
	
	; D3DXMatrixInverse((D3DXMATRIX*)&constantPool[16*4], NULL, (D3DXMATRIX*)&constantPool[16*4]);
	push _constantPool+16*4*4
	push byte 0
	push _constantPool+16*4*4
	call [__imp__D3DXMatrixInverse@12]

	; COMHandles.device->SetRenderTarget(0, COMHandles.renderTarget1.surf);	
	push dword [comhandle(surf1)]
	push byte 0
	comcall [comhandle(device)], SetRenderTarget

%if 0
	; COMHandles.device->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0, 1.0f, 0);	
	push byte 0
	push 0x3f800000
	push byte 0
	push byte D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER
	push byte 0
	push byte 0
	comcall [comhandle(device)], Clear
%endif

	; COMHandles.device->SetPixelShaderConstantF(0, constantPool, 32);	
	push byte NUM_CONSTANTS
	push _constantPool
	push byte 0
	comcall [comhandle(device)], SetPixelShaderConstantF
	
	; COMHandles.device->SetPixelShader(COMHandles.pixelShader2);
	push dword [comhandle(pixelShader2)]
	comcall [comhandle(device)], SetPixelShader
	
	; COMHandles.device->SetTexture(1, COMHandles.renderTarget0.tex);
	push dword [comhandle(tex0)]
	push byte 1
	comcall [comhandle(device)], SetTexture
	
	; COMHandles.device->SetFVF(D3DFVF_XYZRHW|D3DFVF_TEX1);	
	push D3DFVF_XYZRHW|D3DFVF_TEX1
	comcall [comhandle(device)], SetFVF

	; COMHandles.device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, fullscreenQuad, 6*sizeof(float));
	push byte 6*4
	push _fullscreenQuad
	push byte 2
	push byte D3DPT_TRIANGLESTRIP
	comcall [comhandle(device)], DrawPrimitiveUP
	
%if 0
	; COMHandles.device->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
	push byte 0
	push 0x3f800000
	push byte 0
	push byte D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER
	push byte 0
	push byte 0
	comcall [comhandle(device)], Clear
%endif

	; constantPool[PARAM_TIME] += constantPool[PARAM_CAM_SPEED]/24.0f;
	fld  dword [_constantPool+PARAM_TIME*4]
	fadd dword [_invfilmrate]
	fstp dword [_constantPool+PARAM_TIME*4]
	
	; constructMatrix((D3DXMATRIX*)&constantPool[16*4]);
	mov edi, _constantPool+PARAM_CAM*4
	call _create_matrix
	
	; COMHandles.device->SetRenderTarget(0, COMHandles.backbuffer);
	push dword [comhandle(backbuffer)]
	push byte 0
	comcall [comhandle(device)], SetRenderTarget

    %if LETTERBOX==1
	; COMHandles.device->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0, 1.0f, 0);	
	push byte 0
	push 0x3f800000
	push byte 0
	push byte D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER
	push byte 0
	push byte 0
	comcall [comhandle(device)], Clear
	%endif
	
	; COMHandles.device->SetPixelShaderConstantF(0, constantPool, 32);	
	push byte NUM_CONSTANTS
	push _constantPool
	push byte 0
	comcall [comhandle(device)], SetPixelShaderConstantF
	
	; COMHandles.device->SetPixelShader(COMHandles.pixelShader3); 
	push dword [comhandle(pixelShader3)]
	comcall [comhandle(device)], SetPixelShader
	
	; COMHandles.device->SetTexture(2, COMHandles.renderTarget1.tex);
	push dword [comhandle(tex1)]
	push byte 2
	comcall [comhandle(device)], SetTexture
		
	; COMHandles.device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, fullscreenQuad, 6*sizeof(float));
	push byte 6*4
    %if LETTERBOX==1
	push _fullscreenQuad2
	%else
	push _fullscreenQuad
	%endif
	push byte 2
	push byte D3DPT_TRIANGLESTRIP
	comcall [comhandle(device)], DrawPrimitiveUP
		
	; COMHandles.device->EndScene();
	comcall [comhandle(device)], EndScene
	
	; COMHandles.device->Present(0, 0, 0, 0);	
	push byte 0
	push byte 0
	push byte 0
	push byte 0
	comcall [comhandle(device)], Present
	
	popa
	ret