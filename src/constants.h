;/*
%define static
%define const
%define int
%if 0
;*/
#define equ =
#define PLAY_MUSIC

;/*
%endif
;*/


static const int WIDTH				equ	1920;
static const int SHEIGHT            equ 1080;
static const int HEIGHT				equ	1080;
static const int LETTERBOX			equ 0;
static const int BPM				equ	127;
static const int ROWS_PER_BEAT		equ	1;
static const int SAMPLE_RATE		equ	44100;
static const int BYTES_PER_SECOND	equ	(SAMPLE_RATE*2*2);
static const int TRES				equ	256;
static const int INTRO_EXIT_SAMPLE	equ	((9503040)&0xFFFF0000);
static const int MAX_NOTES			equ	8;
static const int FILMRATE			equ 24;


static const int PARAM_CAM_SEEDX				equ		0;
static const int PARAM_CAM_SEEDY				equ		1;
static const int PARAM_CAM_SPEED				equ		2;
static const int PARAM_CAM_FOV					equ		3;
static const int PARAM_CAM_POSY					equ		4;
static const int PARAM_CAM_TARY					equ		5;

static const int PARAM_SUN_ANGLE				equ		6;	
static const int PARAM_WATER_LEVEL				equ		7;
static const int PARAM_SEASON					equ		8;
static const int PARAM_BRIGHTNESS				equ		9;
static const int PARAM_CONTRAST					equ		10;
static const int PARAM_TERRAIN_SCALE			equ		11;

static const int NUM_ROCKET_PARAMS				equ		12;

static const int PARAM_SUNDIR					equ		12;
static const int PARAM_SUNDIR_X					equ		12;
static const int PARAM_SUNDIR_Y					equ		13;
static const int PARAM_SUNDIR_Z					equ		14;
static const int PARAM_TIME						equ		15;



static const int PARAM_CAM						equ		16;
static const int PARAM_CAM_X					equ     16;
static const int PARAM_CAM_Y					equ     17;
static const int PARAM_CAM_Z					equ     18;
static const int PARAM_CAM_ROLL					equ     19;

static const int PARAM_INSTRUMENT_SYNC			equ		20;

static const int PARAM_FILEMTEMPO               equ		23;

static const int NUM_CONSTANTS					equ		64;