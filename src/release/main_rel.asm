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


bits 32

%include "src\constants.h"
%include "src\comcall.nh"

	;; User defs
	global	_WinMainCRTStartup
	global _tinyentry
	extern	_WaveFormat
	extern	_WaveHdr
	extern _generateMusic@0
	global	__fltused
	extern	__imp__CreateWindowExA@48
	extern	__imp__Direct3DCreate9@4
	extern	__imp__waveOutOpen@24
	extern	__imp__waveOutPrepareHeader@12
	extern	__imp__waveOutWrite@12
	extern	__imp__waveOutGetPosition@12
	extern	__imp__ShowCursor@4
	extern	__imp__GetAsyncKeyState@4
	extern	__imp__ExitProcess@4

	;extern	?DemoEffect@@YAXHH@Z
	extern	_demoinit_asm@0
	extern	_demoeffect_asm
	
%define WS_EX_TOPMOST	8
%define WS_POPUP		0x80000000
%define WS_VISIBLE		0x10000000
%define WM_PAINT		0x0F
%define D3DCREATE_HARDWARE_VERTEXPROCESSING	 0x40
%define D3DFMT_D24X8	77

;%define WINDOWED

	section	d3dPrPar data align=1
__fltused:
D3DPresentParams:
	DD	WIDTH
	DD	SHEIGHT
	DD	015H
	DD	02H
	DD	0
	DD	00H
	DD	01H
	DD	00H
%ifdef WINDOWED
	DD	01H	;Windowed
%else
	DD	00H	;Windowed
%endif
	DD	1
	DD	D3DFMT_D24X8	;depth
	DD	00H
	DD	00H
	DD	01H
	

	section	edit data align=1
EditStr:
	db	"edit",0
	
	section	Main text align=1
_tinyentry:
	
_WinMainCRTStartup:

	push	comhandle(device)
	push	D3DPresentParams
	push	byte D3DCREATE_HARDWARE_VERTEXPROCESSING

	push	byte 0
	push	byte 0
	push	byte 0
	push	byte 0
%ifdef WINDOWED
	push	SHEIGHT
	push	WIDTH
%else
	push	byte 0
	push	byte 0
%endif
	push	byte 0
	push	byte 0
	push	byte 0
	push	byte 0
	push	EditStr
	push	byte 0
	call	[__imp__CreateWindowExA@48]
	push	eax						; window

	push	byte 1					; D3DDEVTYPE_HAL
	push	byte 0					; D3DADAPTER_DEFAULT
	
	push	byte 32					; D3D_SDK_VERSION
	call	[__imp__Direct3DCreate9@4]
	
	push	eax
	mov		eax, [eax]
	call	[dword eax+64]				; CreateDevice
	
	call	_demoinit_asm@0

	call	_generateMusic@0
	
	push	byte 0
	call	[__imp__ShowCursor@4]

.waveOutBegin:

; 83   : 
; 84   : 	waveOutOpen(&waveOutHandle, WAVE_MAPPER, &waveformat, 0, 0, 0);

	push	byte 2					; waveOutHandle placeholder
	mov		ebx, esp
	push	byte 0
	push	byte 0
	push	byte 0
	push	_WaveFormat
	push	byte -1
	push	ebx
	call	[__imp__waveOutOpen@24]
	pop		ebp					; waveOutHandle

; 85   : 	waveOutPrepareHeader(waveOutHandle, &wavehdr, sizeof(WAVEHDR));

	push	byte 32					; sizeof(WAVEHDR)
	push	_WaveHdr
	push	ebp						; waveOutHandle
	call	[__imp__waveOutPrepareHeader@12]

; 86   : 	waveOutWrite(waveOutHandle, &wavehdr, sizeof(WAVEHDR));

	push	byte 32					; sizeof(WAVEHDR)
	push	_WaveHdr
	push	ebp
	call	[__imp__waveOutWrite@12]

; 88   : 	do {
; 89   : 		MMTIME playpos;
; 90   : 		playpos.wType = TIME_SAMPLES;
; 91   : 		waveOutGetPosition(waveOutHandle, &playpos, sizeof(MMTIME));

	push	byte 2					; TIME_SAMPLES
	mov		ebx, esp
.mainloop:
	push	byte 32					; sizeof(MMTIME)
	push	ebx
	push	ebp
	call	[__imp__waveOutGetPosition@12]
	
	cmp		dword [esp+4], INTRO_EXIT_SAMPLE
	jge		.exit

	;call	?DemoEffect@@YAXHH@Z
	call	_demoeffect_asm
	
	push	byte 27					; VK_ESCAPE
	call	[__imp__GetAsyncKeyState@4]
	
	test	ax, ax
	je		.mainloop
.exit:
	call	[__imp__ExitProcess@4]