#ifndef _SYNTH_H_
#define _SYNTH_H_

#include "music.h"

typedef struct T_NOTE {
	int dsamples;
	//int count;
} note;

extern "C" {
	extern unsigned char MusicBuffer[TOTAL_SAMPLES*8*MAX_STACK_HEIGHT];
	void __stdcall generateMusic();
	float __stdcall frandom();
	extern int random_seed;
	//extern note notes[];
	extern unsigned char sequence_data[];
	extern unsigned char pattern_data[];
};

#endif