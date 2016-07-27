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

#include <windows.h>
#include <stdio.h>

#include "..\d3d9\d3d9.h"
#include "..\d3d9\d3dx9.h"
#include "..\d3d9\dsound.h"

#include "..\comcall.h"
#include "..\constants.h"
#include "..\synth.h"
#include "sync.h"

bool DemoInit( void );
void DemoEffect( int dummy, int position );

extern "C" 
{
	extern WAVEFORMATEX WaveFormat;
	extern WAVEHDR WaveHdr;
	extern unsigned char MusicBuffer[];
	extern COMStruct COMHandles;
};


static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) 
{
	if(msg == WM_CLOSE) 
    {
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

static D3DPRESENT_PARAMETERS d3dparam = 
{
	WIDTH, SHEIGHT,
	D3DFMT_A8R8G8B8,
	0,//2, // BackBufferCount
	D3DMULTISAMPLE_NONE, 0,
	D3DSWAPEFFECT_DISCARD,
	NULL, // hDeviceWindow
	false, // Windowed
	TRUE, // EnableAutoDepthStencil
	D3DFMT_D24X8, // AutoDepthStencilFormat
	0, // Flags
	0, // RefreshRateInHz
	//D3DPRESENT_INTERVAL_ONE // PresentationInterval
	D3DPRESENT_INTERVAL_IMMEDIATE
};

static LPDIRECTSOUNDBUFFER g_buffer;

static bool InitDirectSound(HWND hwnd) 
{
	DSBUFFERDESC bufferDesc;
	LPDIRECTSOUND ds;
	LPDIRECTSOUNDBUFFER primary;

	if(FAILED(DirectSoundCreate(NULL, &ds, 0)))
		return false;
	
	if(FAILED(ds->SetCooperativeLevel(hwnd, /*DSSCL_EXCLUSIVE |*/ DSSCL_PRIORITY)))
		return false;
	
	memset(&bufferDesc, 0, sizeof(DSBUFFERDESC));
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;//DSBCAPS_STICKYFOCUS;
	bufferDesc.dwBufferBytes = 0;
	bufferDesc.lpwfxFormat = NULL;
	if(FAILED(ds->CreateSoundBuffer(&bufferDesc, &primary, NULL)))
		return false;
	
	if(FAILED(primary->SetFormat(&WaveFormat)))
		return false;
	
	memset(&bufferDesc,0,sizeof(bufferDesc));
	bufferDesc.dwSize = sizeof(bufferDesc);
	bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS| DSBCAPS_GETCURRENTPOSITION2,// | DSBCAPS_STICKYFOCUS;
	bufferDesc.dwBufferBytes = WaveHdr.dwBufferLength;
	bufferDesc.lpwfxFormat = &WaveFormat;
	if(FAILED(ds->CreateSoundBuffer(&bufferDesc, &g_buffer, NULL)))
		return false;
	

	void* p1, *p2;
	DWORD l1, l2;
	if(FAILED(g_buffer->Lock(0, WaveHdr.dwBufferLength, &p1, &l1, &p2, &l2,0)))
		return false;

	memcpy(p1, WaveHdr.lpData, WaveHdr.dwBufferLength);

	if(FAILED(g_buffer->Unlock(p1, l1, p2, l2)))
		return false;

    if(FAILED(g_buffer->Play(0, 0, 0)))
        return false;

	return true;
}

extern "C" _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;

int main( void ) 
{
	return WinMain(0,0,0,0);
}

int WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmd, int ncmd) 
{
	d3dparam.Windowed = TRUE;

    WNDCLASSEX wc;
    memset( &wc, 0, sizeof(WNDCLASSEX) );
	wc.cbSize = sizeof(WNDCLASSEX),			//cbSize;
    wc.lpszClassName = "elevated_class";
    wc.lpfnWndProc   = WndProc;
	wc.hbrBackground = (HBRUSH)CreateSolidBrush(0x00000000);
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hInstance     = inst;
	RegisterClassEx(&wc);

	RECT rec;
	rec.left   = 0;
    rec.top    = 0;
    rec.right  = WIDTH;
    rec.bottom = SHEIGHT;

    DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    DWORD dwStyle = WS_VISIBLE | WS_CAPTION | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_MINIMIZEBOX | WS_SYSMENU;

    AdjustWindowRect( &rec, dwStyle, 0 );

    HWND window = CreateWindowEx( dwExStyle, wc.lpszClassName, "elevated / rgba & tbc",
                                  dwStyle, 0,  0, rec.right-rec.left, rec.bottom-rec.top, 
                                  0, 0, inst, 0 );

    IDirect3D9 *d3d9 = Direct3DCreate9(D3D_SDK_VERSION);

    d3d9->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
                        window, D3DCREATE_HARDWARE_VERTEXPROCESSING,
                        &d3dparam, &COMHandles.device);


	if( !DemoInit() )
    {
		MessageBox( 0, "Could not init demo - shaders?",0,0);
        ExitProcess(1);
    }

	//generate/load music
    #ifdef PLAY_MUSIC
	FILE * file = fopen("music.raw", "rb");
    if( !file )
    {
	    generateMusic();
	    file = fopen("music.raw", "wb");
	    fwrite(MusicBuffer, TOTAL_SAMPLES*4, 1, file);
	    fclose(file);
    }
    else
    {
	    fseek(file, 0, SEEK_END);
	    static int filesize = ftell(file);
	    fseek(file, 0, SEEK_SET);
	    fread(MusicBuffer, filesize, 1, file);
	    fclose(file);
    }
    #endif

	if( !InitDirectSound(window) ) 
    {
		MessageBox(0,"DirectSound initialization failed",0,0);
		ExitProcess(1);
	}

	ShowCursor(0);

	MSG	msg;

	LARGE_INTEGER  tps;    QueryPerformanceFrequency(&tps);
    LARGE_INTEGER  fps_to; QueryPerformanceCounter(&fps_to);
	int numframes = 0;

	bool done = false;
	while( !done )
    {
		if( PeekMessage(&msg, NULL, 0,0, PM_REMOVE) )
        {
			if (msg.message==WM_QUIT) 
            {
				done = true;
			}
            else 
            {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		} 
        else 
        {
			DWORD pos = 0;
			if (GetAsyncKeyState(VK_ESCAPE)) 
            {
				done = true;
			}       
            else 
            {
                g_buffer->GetCurrentPosition( &pos, 0);
                pos = pos / 4;
				DemoEffect(0, pos);
			}

			
            {
				LARGE_INTEGER time; QueryPerformanceCounter(&time);
				numframes++;

				const float dtime = (time.QuadPart - fps_to.QuadPart) / (float)tps.QuadPart;
				if(dtime > 0.2f) 
                {
					fps_to.QuadPart = time.QuadPart;
					char buff[64];
					sprintf(buff,  "%3.2fs  -  %4.1f fps", pos/(float)SAMPLE_RATE, numframes/dtime);
					SetWindowText(window, buff);
					numframes = 0;
				}	
			}
		}
	}

	ExitProcess(0);
}
