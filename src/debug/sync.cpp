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

#include <string.h>
#include <math.h>
#include "sync.h"
#include "../constants.h"
#include "../music.h"

typedef struct
{
    int row;
    float value;
    int interpolation;
}TrackData;

// The data represents the script
// It was generated with "gnu-rocket" by Kusma and Skrebbel (http://rocket.sourceforge.net)
//
// This data got compressed for the Release version of the intro and dumped into
// the file timeline.sync and gets decompressed in demo_rel.asm, line 383

static TrackData camSeedX[] = {
{ 0, 98.000000f, 0},
{ 16, 5.000000, 0},
{ 32, 17.000000f, 0},
{ 44, 113.000000f, 0},
{ 56, 108.000000f, 0},
{ 62, 18.000000f, 0},
{ 72, 9.000000f, 0},
{ 80, 105.000000f, 0},
{ 88, 6.000000f, 0},
{ 92, 101.000000f, 0},
{ 104, 186.000000f, 0},
{ 120, 12.000000f, 0},
{ 140, 81.000000f, 0},
{ 150, 98.000000f, 0},
{ 168, 153.000000f, 0},
{ 196, 114.000000f, 0},
{ 212, 48.000000f, 0},
{ 228, 83.000000f, 0},
{ 260, 11.000000f, 0},
{ 268, 8.000000f, 0},
{ 276, 22.000000f, 0},
{ 292, 11.000000f, 0},
{ 308, 3.000000f, 0},
{ 328, 9.000000f, 0},
{ 344, 50.000000f, 0},
{ 360, 1.000000f, 0},
{ 392, 125.000000f, 0},
{512}
};

static TrackData camSeedY[] = {
{ 0, 0.000000f, 0},
{ 150, 1.000000f, 0},
{ 308, 0.000000f, 0},
{ 344, 1.000000f, 0},
{ 360, 0.000000f, 0},
{512}
};

static TrackData camSpeed[] = {
{ 0, 1.000000f, 0},
{ 92, 5.000000f, 0},
{ 104, 4.000000f, 0},
{ 140, 24.000000f, 0},
{ 150, 58.000000f, 0},
{ 168, 87.000000f, 0},
{ 196, 255.000000f, 0},
{ 228, 188.000000f, 0},
{ 260, 255.000000f, 0},
{ 292, 16.000000f, 0},
{ 308, 64.000000f, 0},
{ 328, 179.000000f, 0},
{ 360, 226.000000f, 0},
{ 392, 30.000000f, 0},
{512}
};

static TrackData camFov[] = {
{ 0, 53.000000f, 0},
{ 16, 160.000000f, 0},
{ 26, 8.000000f, 0},
{ 62, 4.000000f, 0},
{ 75, 2.000000f, 0},
{ 80, 20.000000f, 0},
{ 83, 12.000000f, 0},
{ 88, 8.000000f, 0},
{ 92, 60.000000f, 0},
{ 120, 24.000000f, 0},
{ 140, 18.000000f, 0},
{ 150, 28.000000f, 0},
{ 168, 48.000000f, 0},
{ 196, 160.000000f, 0},
{ 212, 120.000000f, 0},
{ 228, 64.000000f, 0},
{ 260, 128.000000f, 0},
{ 292, 53.000000f, 0},
{ 328, 120.000000f, 0},
{512}
};

static TrackData camPosY[] = {
{ 0, 4.000000f, 0},
{ 16, 128.000000f, 0},
{ 26, 9.000000f, 0},
{ 32, 4.000000f, 0},
{ 44, 5.000000f, 0},
{ 72, 14.000000f, 0},
{ 88, 32.000000f, 0},
{ 92, 8.000000f, 0},
{ 140, 80.000000f, 0},
{ 150, 140.000000f, 0},
{ 168, 16.000000f, 0},
{ 196, 8.000000f, 0},
{ 268, 4.000000f, 0},
{ 276, 16.000000f, 0},
{ 300, 48.000000f, 0},
{ 308, 190.000000f, 0},
{ 328, 14.000000f, 0},
{ 344, 20.000000f, 0},
{ 360, 14.000000f, 0},
{512}
};

static TrackData camTarY[] = {
{ 0, 32.000000f, 0},
{ 16, 255.000000f, 0},
{ 26, 128.000000f, 0},
{ 72, 127.000000f, 0},
{ 88, 128.000000f, 0},
{ 140, 106.000000f, 0},
{ 150, 108.000000f, 0},
{ 168, 115.000000f, 0},
{ 196, 128.000000f, 0},
{ 268, 200.000000f, 0},
{ 276, 128.000000f, 0},
{ 300, 111.000000f, 0},
{ 308, 80.000000f, 0},
{ 344, 100.000000f, 0},
{ 360, 120.000000f, 0},
{512}
};

static TrackData sun_angle[] = {
{ 0, 64.000000f, 0},
{ 26, 90.000000f, 0},
{ 32, 32.000000f, 0},
{ 62, 56.000000f, 0},
{ 72, 160.000000f, 0},
{ 80, 64.000000f, 0},
{ 88, 160.000000f, 0},
{ 92, 180.000000f, 0},
{ 104, 140.000000f, 0},
{ 120, 165.000000f, 0},
{ 140, 110.000000f, 0},
{ 150, 80.000000f, 0},
{ 168, 105.000000f, 0},
{ 196, 50.000000f, 0},
{ 228, 10.000000f, 0},
{ 260, 150.000000f, 0},
{ 276, 85.000000f, 0},
{ 292, 64.000000f, 0},
{ 308, 170.000000f, 0},
{ 328, 100.000000f, 0},
{ 344, 170.000000f, 0},
{ 360, 0.000000f, 0},
{ 392, 35.000000f, 0},
{512}
};

static TrackData terWaterLevel[] = {
{ 0, 154.000000f, 0},
{ 26, 200.000000f, 0},
{ 32, 0.000000f, 0},
{ 72, 170.000000f, 0},
{ 92, 0.000000f, 0},
{ 168, 120.000000f, 0},
{ 196, 160.000000f, 0},
{ 212, 40.000000f, 0},
{ 308, 180.000000f, 0},
{ 344, 0.000000f, 0},
{ 360, 193.000000f, 0},
{ 392, 170.000000f, 0},
{512}
};

static TrackData terSeason[] = {
{ 0, 0.000000f, 0},
{ 292, 0.000000f, 1},
{ 300, 64.000000f, 1},
{ 308, 128.000000f, 1},
{ 322, 255.000000f, 0},
{ 392, 255.000000f, 1},
{ 424, 0.000000f, 0},
{512}
};

static TrackData imgBrightness[] = {
{ 0, 0.000000f, 1},
{ 8, 128.000000f, 0},
{ 26, 110.000000f, 0},
{ 62, 32.000000f, 0},
{ 72, 90.000000f, 0},
{ 92, 110.000000f, 0},
{ 120, 128.000000f, 0},
{ 140, 90.000000f, 0},
{ 160, 90.000000f, 1},
{ 167, 0.000000f, 0},
{ 168, 128.000000f, 0},
{ 196, 120.000000f, 0},
{ 228, 105.000000f, 0},
{ 250, 105.000000f, 1},
{ 251, 128.000000f, 0},
{ 260, 100.000000f, 0},
{ 308, 24.000000f, 0},
{ 328, 120.000000f, 0},
{ 360, 110.000000f, 0},
{ 392, 100.000000f, 0},
{ 424, 100.000000f, 1},
{ 448, 0.000000f, 0},
{512}
};

static TrackData imgContrast[] = {
{ 0, 150.000000f, 0},
{ 62, 250.000000f, 0},
{ 72, 180.000000f, 0},
{ 92, 0.000000f, 1},
{ 102, 160.000000f, 0},
{ 120, 128.000000f, 0},
{ 140, 190.000000f, 0},
{ 160, 190.000000f, 1},
{ 167, 130.000000f, 0},
{ 168, 160.000000f, 0},
{ 196, 140.000000f, 0},
{ 228, 180.000000f, 0},
{ 292, 0.000000f, 1},
{ 293, 190.000000f, 0},
{ 308, 255.000000f, 0},
{ 328, 150.000000f, 0},
{ 360, 170.000000f, 0},
{ 392, 180.000000f, 0},
{ 424, 180.000000f, 1},
{ 448, 128.000000f, 0},
{512}
};

static TrackData terScale[] = {
{ 0, 200.000000f, 0},
{ 26, 140.000000f, 0},
{ 32, 200.000000f, 0},
{ 120, 255.000000f, 0},
{ 260, 220.000000f, 0},
{ 292, 255.000000f, 0},
{ 328, 20.000000f, 0},
{ 360, 230.000000f, 0},
{512}
};




float SyncParam( int position, char *track )
{
    const TrackData *td = camSeedX;

	if( strcmp(track,"camSeedX")==0 ) td = camSeedX;
	if( strcmp(track,"camSeedY")==0 ) td = camSeedY;
	if( strcmp(track,"camSpeed")==0 ) td = camSpeed;

	if( strcmp(track,"camFov")==0 ) td = camFov;
	if( strcmp(track,"camPosY")==0 ) td = camPosY;
	if( strcmp(track,"camTarY")==0 ) td = camTarY;
	if( strcmp(track,"sun_angle")==0 ) td = sun_angle;
	if( strcmp(track,"terWaterLevel")==0 ) td = terWaterLevel;
	if( strcmp(track,"terSeason")==0 ) td = terSeason;

	if( strcmp(track,"imgBrightness")==0 ) td = imgBrightness;
	if( strcmp(track,"imgContrast")==0 ) td = imgContrast;
	if( strcmp(track,"terScale")==0 ) td = terScale;

    const float ro = float(position) / float(MAX_NOTE_SAMPLES*4);
    const float ir = floorf(ro);
    const int   iri = int(ir);
    
    int r = 0;
    for( r=0; r<100; r++ )
    {
        if( td[r].row >= iri ) break;
    }
    if( r>0 ) r--;
    if( td[r].interpolation==0 ) return td[r].value;

    return td[r].value + (td[r+1].value-td[r].value) * (ro-td[r].row)/float(td[r+1].row-td[r].row);
}
