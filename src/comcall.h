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


#ifndef _COMCALL_H_
#define _COMCALL_H_

#include "d3d9\d3d9.h"
#include "d3d9\d3dx9mesh.h"


struct texture_surface_pair 
{
	LPDIRECT3DTEXTURE9 tex;
	LPDIRECT3DSURFACE9 surf;
};

struct COMStruct
{
	LPDIRECT3DDEVICE9			device;
	void*						dummy0;
	LPDIRECT3DVERTEXSHADER9		vertexShader0;
	void*						dummy1;
	LPDIRECT3DPIXELSHADER9		pixelShader0;
	void*						dummy2;
	LPDIRECT3DPIXELSHADER9		pixelShader1;
	void*						dummy3;
	LPDIRECT3DPIXELSHADER9		pixelShader2;
	void*						dummy4;
	LPDIRECT3DPIXELSHADER9		pixelShader3;
	texture_surface_pair		renderTarget0;
	texture_surface_pair		renderTarget1;
	LPDIRECT3DSURFACE9			camRenderTarget;
	LPDIRECT3DSURFACE9			backbuffer;
	LPD3DXMESH					mesh;
};

extern "C" {
	extern COMStruct COMHandles;
};

#endif