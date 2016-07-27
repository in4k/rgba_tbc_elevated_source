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

#pragma once

#include "constants.h"
#include "synth.h"
#include "d3d9\d3dx9.h"
#include "comcall.h"

extern "C" 
{

float fullscreenQuad[] = { 0,      0,      0, 1, 0, 0,
	                       WIDTH,  0,      0, 1, 1, 0,
	                       0,	   HEIGHT, 0, 1, 0, 1,
                           WIDTH,  HEIGHT, 0, 1, 1, 1 };	

float fullscreenQuad2[] = { 0,     (SHEIGHT-HEIGHT)/2, 0, 1, 0, 0,
                            WIDTH, (SHEIGHT-HEIGHT)/2, 0, 1, 1, 0,
                            0,     (SHEIGHT+HEIGHT)/2, 0, 1, 0, 1,
                            WIDTH, (SHEIGHT+HEIGHT)/2, 0, 1, 1, 1 };	

float constantPool[256];

unsigned char MusicBuffer[TOTAL_SAMPLES*8*MAX_STACK_HEIGHT];

void WINAPI TextureFillCallback(D3DXVECTOR4* pOut, CONST D3DXVECTOR2* pTexCoord,  CONST D3DXVECTOR2* pTexelSize,  LPVOID pData)
{
	float f = frandom();
	pOut->x = f;
}


#pragma bss_seg(".comhnd")
COMStruct COMHandles;


#pragma data_seg(".shader")
char ubershader[] = ""
    "sampler t0,t1,t2;"
    "float4 q[16];"		//q parameters, syncdata
    "float4x4 v:register(c16);"

    // this is a value noise function which also computes derivatives. It's not really optimized
    // see here: http://www.iquilezles.org/www/articles/morenoise/morenoise.htm
    "float3 no(float2 p)"
    "{"
        "float2 f=p-floor(p);"
        "float2 u=f*f*f*(f*(f*6-15)+10);"
        "float a=tex2Dlod(t0,float4((floor(p)+float2(0,0))/256,0,0));"
        "float b=tex2Dlod(t0,float4((floor(p)+float2(1,0))/256,0,0));"
        "float c=tex2Dlod(t0,float4((floor(p)+float2(0,1))/256,0,0));"
        "float d=tex2Dlod(t0,float4((floor(p)+float2(1,1))/256,0,0));"
        "return float3(a+(b-a)*u.x+(c-a)*u.y+(a-b-c+d)*u.x*u.y,30*f*f*(f*(f-2)+1)*(float2(b-a,c-a)+(a-b-c+d)*u.yx));"
    "}"

    // procedural terrain computation. Based on noise derivatives to simulate some erosion.
    "float f(float2 p,float o)"
    "{"
        "float2 d=0;"
        "float a=0;"
        "float b=3;"
        "for(float i=0;i<o;i++)"
        "{"
            "float3 n=no(.25*p);"
            "d+=n.yz;"
            "a+=(b*=.5)*n.x/(1+mul(d,d));"
            "p=mul(float2x2(1.6,-1.2,1.2,1.6),p);"
        "}"
        "return a;"
	  "}"

    // normal computation bu regular gradient approximation (central differences)
    "float3 cn(float2 p,float e,float o)"
    "{"
        "float a=f(p,o);"
        "return normalize(float3(q[2].w*(a-f(p+float2(e,0),o)),e,q[2].w*(a-f(p+float2(0,e),o))));" //iq
    "}"

    // lighting. p=position, c=normal, d=smooth normal (for faking shadows)
    "float3 b(float3 p,float3 c,float3 d)"
    "{"
        "float a=mul(d,q[3]);"							//sdl
        "float b=lerp(a,mul(c,q[3]),.5+.5*q[2].x);"		//ndl
        "return float3(.13,.18,.22)*(c.y+.25*saturate(-b)-.1*no(1024*p.xz).y)+" // amb
               "float3(1.4,1,.7)*saturate(b)*saturate(2*a);"			// dif
    "}"

    "float4 m0(float4 x:position,out float4 y:color):position"
    "{"
        "x.z=q[2].w*f(x.yx,8);"
        "y=x.yzxw;"			// .w must be > 0.5
        "return mul(v,y);"  // iq
    "}"

	// camera shader - x.x = 0 for cam x.x = 1 for target

    "float4 m1(float2 x:vpos):color"
    "{"
        "float2 o=q[0]+x.x*.37;"
        "float3 c;"
        // move
        "float t=q[3].w*q[0].z;"
        "c.x=16*cos(t*tex2D(t0,o+=.1)+3*tex2D(t0,o+=.1))+8*cos(t*tex2D(t0,o+=.1)*2+3*tex2D(t0,o+=.1));"
        "c.z=16*cos(t*tex2D(t0,o+=.1)+3*tex2D(t0,o+=.1))+8*cos(t*tex2D(t0,o+=.1)*2+3*tex2D(t0,o+=.1));"
        // collide
        "c.y=q[2].w*f(c.xz,3)+q[1].x+q[1].y*x.x;"
        // shake
        "o+=q[3].w*.5;"
        "c.x+=.002*no(o+=.1);"
        "c.y+=.002*no(o+=.1);"
        "c.z+=.002*no(o+=.1);"

        // pos+roll
        "return float4(c,.3*cos(t*2));"
    "}"

    "float4 m2(float4 y:color):color" // psmain0
    "{"
        "return y;"
    "}"

    "float4 m3(float2 x:texcoord):color"	// psmain1
    "{"
        "float2 o=x+.5/1280;"
        "float4 d=tex2D(t1,o);"											// dat
        "float3 e=normalize(mul(v,float4(x.x*2-1,-x.y*2+1,1,1)));"		// eye
        "float2 s=e.xz/e.y;"
        "float k=(2*s.y+1000)%8;"

		// sky
		    // dome
            "float3 c=float3(.55,.65,.75)"	
            // clouds
            "+.1*f(s+q[3].w*.2,10)"
            // horizon
            "+.5*pow(1-e.y,8)"
            // sun
            "+pow(saturate(mul(e,q[3])),16)*float3(.4,.3,.1)"
			// melody rays
			  "+float4(1+.4*k,2,3+.5*k,0)*(1-cos(12.5664*s.y))*saturate(1-abs(s.y)/10-abs(s.x+q[5+k].x*.0012-8)/20)*exp(-q[5+k].x*.0002);"

        // terrain texturing and lighting
        "if(d.w>.5)"
        "{"
            "float t=length(d.xyz-q[4].xyz);"

            // check for water plane
            "float w=q[1].w-d.y;"
            "if(w<0)"
            "{"
                // compute normal
                "float3 n=cn(d.xz,.001*t,12-log2(t));"
                // compute albedo texture
                "float h=f(3*d.xz,3);"
                "float r=no(666*d.xz);"
                //   rock
                "c=(.1+.75*q[2].x)*(.8+.2*r);"
                //   grass 1
                "c=lerp(c,lerp(float3(.8,.85,.9),float3(.45,.45,.2)*(.8+.2*r),q[2].x),smoothstep(.5-.8*n.y,1-1.1*n.y,h*.15));"
                //   grass2
                "c=lerp(c,lerp(float3(.37,.23,.08),float3(.42,.4,.2),q[2].x)*(.5+.5*r),smoothstep(0,1,50*(n.y-1)+(h+q[2].x)/.4));"
                // apply lighting
                "c*=b(d,n,cn(d.xz,.001*t,5));"

            "}"
            "else"
            "{"
                // intersect water plane
                "t=(q[1].w-q[4].y)/e.y;"
                "d=q[4]+e.xyzz*t;"

                //compute some normal
//               "float3 n=cn(float2(512,32)*d.xz+saturate(w*60)*float2(q[3].w,0),.001*t,4);",  "n.y*=6;",   "n=normalize(n);"
                "float3 n=normalize(cn(float2(512,32)*d.xz+saturate(w*60)*float2(q[3].w,0),.001*t,4)*float3(1,6,1));"

                // water color based on depth
                "c=.12*(float3(.4,1,1)-float3(.2,.6,.4)*saturate(w*16));" // iq
                // darken water in winter scene
                "c*=.3+.7*q[2].x;"
                // reflections
                "c+=pow(1-mul(-e,n),4)*(pow(mul(q[3],reflect(-e,n)),32)*float3(.32,.31,.3)+.1);"
                // snow in the lake borders (water,snow,foam)
                "c=lerp(c,b(d,n,n),smoothstep(1,0,q[2].x+w*60-f(666*d.xz+saturate(w*60)*float2(q[3].w,0)*2,5))*.5);"
            "}"
            // fake some AO in the water/land contact area
            "c*=.7+.3*smoothstep(0,1,256*abs(w));"

            // fog/scatt:
            // extintion
            "c*=exp(-.042*t);"
            // inscattering
            "c+=(1-exp(-.1*t))*(float3(.52,.59,.65)+pow(saturate(mul(e,q[3])),8)*float3(.6,.4,.1));"
            "}"
        "return float4(c,0);"
        "}"

    "float4 m4(float2 x:texcoord):color"	// psmain2
    "{"
        "float2 o=x+.5/1280;"
        "float4 d=tex2D(t1,o);"
        "float3 c=tex2D(t2,o);"
        "if(d.w>.5)"
        "{"
            // compute velocity vector
            "d=mul(v,float4(d.xyz,1));"
            "d.y*=-1;"
            // blur
            "c=0;"
            "for(float i=0;i<16;i++)"
            "{"
                "c.x+=tex2D(t2,o+i*(.5+.5*d.xy/d.w-o)/16+float2(2,0)/1280).x;"
                "c.y+=tex2D(t2,o+i*(.5+.5*d.xy/d.w-o)/16+float2(0,0)/1280).y;"
                "c.z+=tex2D(t2,o+i*(.5+.5*d.xy/d.w-o)/16+float2(-2,0)/1280).z;"
            "}"
            "c/=16;"
        "}"

        // tonemap
        "c=pow(c,.45)*q[2].z+q[2].y;"
        // vigneting
        "c*=.4+9.6*o.x*o.y*(1-o.x)*(1-o.y);"
        // color ink
        "c.xz*=.98;"
        // light flicker (at 24 hz)
        "float w=tex2D(t0,q[3].w*.1);"	// 24/256=0.0937
        "o+=w;"
        "c-=.005*w;"
		// grain (at 24 hz)
        "c.x+=.01*tex2D(t0,o+=.1);"
        "c.y+=.01*tex2D(t0,o+=.1);"
        "c.z+=.01*tex2D(t0,o+=.1);"
        // dust (at 24 hz)
//        "c-=.02*smoothstep(1.5,1.6,f(o*float2(64,8),5));"

        "return float4(c,0);"
        "}";


int ubershaderLen = sizeof(ubershader)-1;

}
