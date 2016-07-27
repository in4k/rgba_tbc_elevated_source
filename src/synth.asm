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
%include "src\music.asm"
%define STACK_SAMPLES TOTAL_SAMPLES

global _generateMusic@0
global _musiclengths
global _bufferlength
extern _MusicBuffer

%include "src\synth_core.nh"

section parmem bss align=64
_param_memory: resd	TOTAL_PARAM_WORDS			;NUM_MACHINES*PARAMBLOCK_SAMPLES*8

;section playnote bss align=4
;_playnote: resd 1

section synth code align=1
synth:
	add edi, STACK_SAMPLES*8
	
	;clear
	pusha
	xor eax, eax
	mov ecx, STACK_SAMPLES*2
	rep stosd
	popa
	
	pusha
	mov ecx, NUM_ROWS*16
.noteloop:
	pusha
	
	mov eax, edx
	shr eax, byte 4
	movzx eax, byte [_sequence_data+eax]
	shl eax, byte 4
	and edx, byte 0xF
	movzx eax, byte [_pattern_data+eax+edx]
	
	;instrument note renderer
	;---------------------------
	;dword: attack
	;dword: decay
	;dword: sustain
	;dword: release
	;float: noise mix
	;float: freq_exp
	;float: base_freq
	;float: volume
	;float: stereo
	;osc data: dd*3*3
	;---------------------------
	;ebp: instrument definition
	;eax: note
	
	add al, al
	jz near .exit_renderloop
	
	mov esi, envelope_scales
	cmp al, byte 0xFE
	jne .playnote
		add esi, byte 16
	.playnote:
		
	xchg eax, ecx
	;ecx: note1

	;calculate note1 frequency
	fld dword [note_freq_start]
.note1_loop:
	fmul dword [note_freq_step]
	loop .note1_loop
	
	;st0: note1 freq
	fsub	dword [ebp+6*4]	;base freq
	fldz	;phase
	fldz	;envelope value
	
	;st2: freq1
	;st1: phase
	;st0: envelope value

	xor edx, edx
.segmentloop:
	mov ecx, dword [ebp+edx*4]
	fld dword [esi+edx*4]
	fidiv dword [ebp+edx*4]
	
	;st3: frequency
	;st2: phase
	;st1: envelope value
	;st0: envelope amplitude step	
.sampleloop:
	fadd st1, st0			;update envelope amplitude
	fld st3					;frequency
	fmul dword [ebp+5*4]	;*= freq_exp
	fst st4					;save new frequency
	fadd st3				;increment phase
	fadd dword [ebp+6*4]	;+base_freq_step
	fstp st3				;store phase

	;;-- osc
	pusha
	lea ebx, [ebp+9*4]
	push byte 3
	pop ecx
	fldz
	
	;st4: frequency
	;st3: phase
	;st2: envelope value
	;st1: envelope value step
	;st0: osc accum
.osc_loop:
	fld st3
	;osc 1
	fld dword [__real2]
	fsub dword [ebx+8]	;detune
	fmulp st1
	
	call osc
	
	;osc 2
	fld st4
	fmul dword [ebx+8]
	call osc
	faddp st1

	movzx eax, byte [ebx+1]
	dec eax
	jne .not_nop
		fstp st0
	.not_nop:
	dec eax
	jne .not_add
		faddp st1
	.not_add:
	dec eax
	jne .not_sub
		fsubp st1
	.not_sub:
	dec eax
	jne .not_mul
		fmulp st1
	.not_mul:
	
	add ebx, byte 12
	loop .osc_loop
	popa
	
	;mix noise and osc
	call _frandom@0
	
	fmul dword [ebp+4*4]
	faddp st1
	
	;st0: osc
	fmul st2	;*= envelope value
	fmul dword [ebp+7*4]
	fst dword [edi]
	scasd
	fmul dword [ebp+8*4]
	fstp dword [edi]
	scasd
	
	loop .sampleloop
	
	fstp st0	;pop envelope step
	
	inc edx
	cmp edx, byte 4
	jne .segmentloop
	
	fstp st0	;pop envelope amp
	fstp st0	;pop phase
	fstp st0	;pop frequency step
	
.exit_renderloop:
	popa
	add edi, MAX_NOTE_SAMPLES*2*4
	inc edx
	
	dec ecx
	jne .noteloop
	
	popa
	add edx, NUM_ROWS*16
	add esi, byte 72
ret	

section machines data align=4
machine_table:
	dd synth				;0
	dd delay				;1
	dd filter_machine		;2
	dd compressor_machine	;3
	dd mixer_machine		;4
	dd distortion_machine2	;5
	;dd distortion_machine	;6
	dd allpass				;7

section syntheng code align=1
_generateMusic@0:
	pusha
	;depack 
	
	xor edx, edx
	xor eax, eax
	
	mov esi, _machine_tree
	mov edi, _MusicBuffer-STACK_SAMPLES*8
	mov ebx, _param_memory
.machineloop:
	mov ecx, TOTAL_SAMPLES
	mov ebp, esi
	call dword [machine_table+eax*4]
	
	lodsb
	test al, al
	jns .machineloop
	
	;final mix
	add ecx, ecx	;mov ecx, TOTAL_SAMPLES*2
	mov edi, _MusicBuffer
	mov esi, edi
.mixloop:
	fld dword [edi]
	fimul dword [final_scale]
	fistp word [esi]
	scasd
	lodsb
	lodsb
	loop .mixloop
	
	popa
	ret