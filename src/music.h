;/*
%define static
%define const
%define int
%if 0
;*/
#define equ =
;/*
%endif
;*/

static const int PARAMBLOCK_SAMPLES equ 327680;
static const int NUM_CHANNELS equ 12;
static const int NUM_MACHINES equ 43;
static const int MAX_DELAY_SAMPLES equ 65536;
static const int MAX_NOTE_LENGTH equ 327680;
static const int MAX_STACK_HEIGHT equ 4;
static const int NUM_ROWS equ 114;
static const int TOTAL_SAMPLES equ ((9503040+65535)&0xFFFF0000);
static const int MAX_NOTE_SAMPLES equ 5210;
static const int TOTAL_PARAM_WORDS equ 1114112;
