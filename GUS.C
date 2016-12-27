/*
===============================================================================
    WICKED PLAYER Gravis UltraSound DRIVER v0.95beta

    Copyright (C) 1997, Maciej Sini뭥 (AKA Yarpen/Swirl).
    Thanks to Technomancer^FireLight.
===============================================================================
*/


#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>
#include "wicked.h"

//#define LOUD_VOLTAB

/* modes */
#define C_VoiceStopped     0x1                  // command (write voice mode)
#define C_StopVoice        0x2
#define C_Data16Bit        0x4
#define C_EnableLoop       0x8
#define C_BiDirectionLoop  0x10
#define C_WaveTableIRQ     0x20
#define C_Direction        0x40
#define C_IRQPending       0x80
#define M_LineIn           0x1                  // mixer control
#define M_Output           0x2
#define M_Mic              0x4
#define M_DMA_IRQ          0x40
#define V_RampStopped      0x1                  // volume control
#define V_StopRamp         0x2
#define V_EnableLoop       0x8
#define V_BiDirectionLoop  0x10
#define V_VolRampIRQ       0x20
#define V_Direction        0x40


/* let's define cool GUS ports */
#define StatusPort       0x6
#define TimerCtrlPort    0x8
#define TimerDataPort    0x9
#define IRQDMAPort       0xB
#define MidiCtrlPort     0x100
#define MidiDataPort     0x101
#define ActiveVoicePort  0x102
#define CommandPort      0x103
#define DataLoPort       0x104
#define DataHiPort       0x105
#define DRAMIOPort       0x107

/* now some cool GUS commands  */
#define WriteVoiceMode   0x0
#define SetVoiceFreq     0x1
#define LoopStartLo      0x2
#define LoopStartHi      0x3
#define LoopEndLo        0x4
#define LoopEndHi        0x5
#define VolRampRate      0x6
#define VolRampStart     0x7
#define VolRampEnd       0x8
#define SetVol           0x9
#define LoopBeginLo      0xA
#define LoopBeginHi      0xB
#define SetBalance       0xC
#define VolCtrl          0xD
#define ActiveVoices     0xE
#define DMACtrl          0x41
#define DRAMAddrLo       0x43
#define DRAMAddrHi       0x44
#define TimerCtrl        0x45
#define TimerSpeed       0x46
#define SampleCtrl       0x49
#define Initialize       0x4C
#define ReadVoiceMode    0x80
#define ReadVoiceFreq    0x81
#define ReadLoopStartLo  0x82
#define ReadLoopStartHi  0x83
#define ReadLoopEndLo    0x84
#define ReadLoopEndHi    0x85
#define ReadVolRampRate  0x86
#define ReadVolRampStart 0x87
#define ReadVolRampEnd   0x88
#define ReadVol          0x89
#define ReadVoicePosLo   0x8A
#define ReadVoicePosHi   0x8B
#define ReadVoiceBalance 0x8C
#define ReadVolCtrl      0x8D
#define ReadActiveVoices 0x8E
#define IRQStatus        0x8F


#define ADDR_LOW(x) ((unsigned int)((unsigned int)((x>>7L)&0x1fffL)))
#define ADDR_HIGH(x)  ((unsigned int)((unsigned int)((x&0x7fL)<<9L)))



/*--------------------------------------------------------------------------*/
/*---------------------------------- DATA ----------------------------------*/
/*--------------------------------------------------------------------------*/
static uword    GUSBase;
static udword   GUSMem;
static udword   GUSDivisor;
uword           GUSMixingSpeed;
/* Volume Table From FMOD [quite loud] */
#ifdef LOUD_VOLTAB
static uword    GUSVol[65] =
{
        0x1500,
        0xA0DE,0xAB52,0xB2BD,0xB87E,0xBD31,0xC12B,0xC49C,0xC7A5,
        0xCA5D,0xCCD2,0xCF10,0xD120,0xD309,0xD4D1,0xD67B,0xD80B,
        0xD984,0xDAE9,0xDC3B,0xDD7D,0xDEB0,0xDFD6,0xE0EF,0xE1FC,
        0xE2FF,0xE3F8,0xE4E8,0xE5D0,0xE6AF,0xE788,0xE859,0xE924,
        0xE9E9,0xEAA9,0xEB63,0xEC18,0xECC8,0xED73,0xEE1A,0xEEBD,
        0xEF5C,0xEFF7,0xF08F,0xF123,0xF1B5,0xF242,0xF2CD,0xF356,
        0xF3DB,0xF45E,0xF4DE,0xF55B,0xF5D7,0xF650,0xF6C7,0xF73C,
        0xF7AE,0xF81F,0xF88E,0xF8FB,0xF967,0xF9D0,0xFA38,0xFA9E
};
#else
/* Volume Table From MIDAS060 [a bit less loud -- I prefer this one] */
static uword GUSVol[65] =
{
              0x1500,0x8f10,0x9f10,0xab50,0xaf10,0xb970,0xbb50,0xbd30,
              0xbf10,0xc880,0xc970,0xca60,0xcb50,0xcc40,0xcd30,0xce20,
              0xcf10,0xd800,0xd880,0xd8f0,0xd970,0xd9e0,0xda60,0xdad0,
              0xdb50,0xdbc0,0xdc40,0xdcb0,0xdd30,0xdda0,0xde20,0xde90,
              0xdf10,0xdf80,0xe800,0xe840,0xe880,0xe8b0,0xe8f0,0xe930,
              0xe970,0xe9a0,0xe9e0,0xea20,0xea60,0xea90,0xead0,0xeb10,
              0xeb50,0xeb80,0xebc0,0xec00,0xec40,0xec70,0xecb0,0xecf0,
              0xed30,0xed60,0xeda0,0xede0,0xee20,0xee50,0xee90,0xeed0,
              0xef00
};
#endif


static char     GUSName[] =
{"Gravis UltraSound Driver v0.97beta. Copyright (C) Yarpen/Swirl ["__DATE__"]"};
static char     GUSDeviceName[] =
{"Gravis UltraSound"};

/* function prototypes */
void GUSDumpSample(udword Loc, char *buffer, uword buflength, ubyte xorval);
static void GUSPlayVoice(ubyte Voice,ubyte Mode,udword begin,udword start,udword end);
static void GUSStopVoice(ubyte Voice);
static void GUSSetVolume(ubyte Voice, ubyte Volume);
static void GUSSetFreq(ubyte Voice, uword Freq);
static void GUSSetBalance(ubyte Voice, ubyte Balance);
static void GUSReset(ubyte NumVoices);
static void GUSFindMem(void);
static int GUSFind(void);
static int GUSCheckEnv(void);
static void GUSVolRamp(ubyte voice, ubyte vol);
static bool GUSInit(void);

/* Gravis driver */
DRIVER          DrvGUS = {
  &GUSName,
  &GUSDeviceName,
  0,                                            /* wavetable */
  0x220,                                        /* standard? */
  0,
  GUSInit,
  GUSReset,
  GUSDumpSample,
  GUSPlayVoice,
  GUSStopVoice,
  GUSVolRamp,
  GUSSetBalance,
  GUSSetFreq,
  0,                                    // maybe some day...:)
  0
};
DRIVER          *UsedDrv;




//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�


/*
=================
GUSInit
=================
*/
static bool GUSInit(void)
{
  if (!GUSCheckEnv())
  {
    printf("ULTRASND Environment String Not Found. Performing Hardware Detection...\n");
    if (!GUSFind())
      return 0;
  }
  DrvGUS.DeviceBase = GUSBase;

  GUSFindMem();
  if (!GUSMem)
    return 0;

  DrvGUS.DeviceMem = GUSMem;

  return 1;
}



/*
=================
GUSDelay

gives time for our GUS to do what it has to do
=================
*/
void GUSDelay(void);
#pragma aux GUSDelay =\
        "push        ax",\
        "push        dx",\
        "mov         dx,0300h",\
        "in          al,dx",\
        "in          al,dx",\
        "in          al,dx",\
        "in          al,dx",\
        "in          al,dx",\
        "in          al,dx",\
        "in          al,dx",\
        "pop         dx",\
        "pop         ax",\

void GUSOutByte(byte index, byte value);
void GUSOutWord(byte index, word value);
#pragma aux GUSOutByte =\
        "mov         dx,GUSBase",\
        "add         dx,103h",\
        "out         dx,al",\
        "add         dl,2",\
        "mov         al,bl",\
        "out         dx,al",\
        parm caller [al] [bl] modify [ax dx];
#pragma aux GUSOutWord =\
        "mov         dx,GUSBase",\
        "add         dx,103h",\
        "out         dx,al",\
        "inc         dl",\
        "mov         ax,bx",\
        "out         dx,ax",\
        parm caller [al] [bx] modify [ax dx];


/*
==============================================================================
GUSCalcFreq

Calculate GUS frequency from period (ST3 period = 4*Amiga period)
First convert period to frequency:

  Freq = 14317056 / period

Precalculated table will be too big (about 27000 periods!)

[08-07-97]
Now it's a bit optimized! Since the final formula for GUS freq looks like:

  GUSFreq = (14317056 / period) / GUSDivisor

We can write it down also as:

  GUSFreq = (14317056 / GUSDivisor) / period

14317056/GUSDivisor = const., so, it could be precalculated. And that is
what I did.
==============================================================================
*/
static uword GUSCalcFreq(uword p)
{
  uword         GUSF;
  udword        temp;

  if (!p)
    return 0;

  //temp = 14317056 / p;                /* convert period to frequency */
  GUSF = GUSDivisor/p;

  return GUSF;
}




/*
==============================================================================
GUSPeek

Read byte from GUS memory at position LOC
==============================================================================
*/
ubyte GUSPeek(udword loc)
{
  GUSOutWord(DRAMAddrLo, loc);
  GUSOutByte(DRAMAddrHi, (loc >> 16) & 0xFF);

  return inp(GUSBase+DRAMIOPort);
}




/*
==============================================================================
GUSPoke

Put WHAT to GUS memory at position LOC
==============================================================================
*/
void GUSPoke(udword loc, ubyte what)
{
  GUSOutWord(DRAMAddrLo, loc);
  GUSOutByte(DRAMAddrHi, (loc >> 16) & 0xFF);

  outp(GUSBase+DRAMIOPort, what);
}




/*
==============================================================================
GUSDumpSample

Kick out sample to GUS memory, a kind of serie of pokes...
LOC = position in GUS mem to put sample at
BUFFER -> sample to kick
BUFLENGTH = guess
XORVAL = value to xor every byte of sample with (signed/unsigned stuff)

Taken almost as is from FireLight's (a.k.a. Brett Paterson) FMOD. Thanx!
==============================================================================
*/
void GUSDumpSample(udword loc, char *buffer, uword buflength, ubyte xorval) {
  register uword si, bx = 0;
  register udword di;

  si = ((loc >> 16) & 0xFF);
  di = (uword)loc;

  GUSOutByte(DRAMAddrHi, (ubyte)si);
MainLoop:
  GUSOutWord(DRAMAddrLo, (uword)di);
  outp(GUSBase+DRAMIOPort, *(buffer+bx)+xorval);
  bx++;
  di++;
  if (di <= 65535)
    goto DoLoop;

  di = 0;
  si++;
  GUSOutByte(DRAMAddrHi, (ubyte)si);
DoLoop:
  if (buflength == 0)
    return;
  buflength--;
  goto MainLoop;
}



/*
==============================================================================
GUSReset

Does exactly what the name says: resets the soundcard named GUS
VOICES = number of voices we will use (at least 14)
==============================================================================
*/
static void GUSReset(ubyte voices)
{
  //uword          GUSDivisors[] = {40, 37, 35, 33, 31, 30, 28, 27, 26, 25,
  //                                24, 23, 22, 21, 20, 20, 19, 18};
  udword         GUSDivisors[] = {14317056/40, 14317056/37, 14317056/35,
                                  14317056/33, 14317056/31, 14317056/28,
                                  14317056/27, 14317056/26, 14317056/25,
                                  14317056/24, 14317056/22, 14317056/21,
                                  14317056/20, 14317056/20, 14317056/19,
                                  14317056/18};
  uword          GUSSpeeds[] =   {40, 37, 35, 33, 31, 30, 28, 27, 26, 25,
                                  24, 23, 22, 21, 20, 20, 19, 18};

  register ubyte i;

  if (voices < 14)
    voices = 14;

  if (voices == 14)
  {
    GUSDivisor = 14317056/43;
    GUSMixingSpeed = 43;
  }
  else
  {
    GUSDivisor = GUSDivisors[Mod.NrChans-15];
    GUSMixingSpeed = GUSSpeeds[Mod.NrChans-15];
  }

  outp(GUSBase, M_Output);
  GUSPoke(0,0);

  GUSOutByte(Initialize, 0);
  GUSDelay();
  GUSDelay();
  GUSOutByte(Initialize, 1);

  GUSDelay();
  GUSDelay();

  GUSOutByte(DMACtrl, 0);
  GUSOutByte(TimerCtrl, 0);
  GUSOutByte(SampleCtrl, 0);

  GUSOutByte(ActiveVoices, (voices-1) | 0xC0);

  inp(GUSBase+StatusPort);
  outp(GUSBase+CommandPort, DMACtrl);
  inp(GUSBase+DataHiPort);
  outp(GUSBase+CommandPort, SampleCtrl);
  inp(GUSBase+DataHiPort);
  outp(GUSBase+CommandPort, IRQStatus);
  inp(GUSBase+DataHiPort);

  for (i = 0; i < 32; i++)
  {
    outp(GUSBase+ActiveVoicePort, i);
    GUSOutByte(WriteVoiceMode, C_VoiceStopped | C_StopVoice);
    GUSOutByte(VolCtrl, V_RampStopped | V_StopRamp);
    GUSOutWord(SetVoiceFreq, 0);
    GUSOutWord(LoopStartLo, 0);
    GUSOutWord(LoopStartHi, 0);
    GUSOutWord(LoopEndLo, 0);
    GUSOutWord(LoopEndHi, 0);
    GUSOutWord(VolRampRate, 0);
    GUSOutWord(VolRampStart, 0);
    GUSOutWord(VolRampEnd, 0);
    GUSOutWord(LoopBeginLo, 0);
    GUSOutWord(LoopBeginHi, 0);
    GUSOutWord(SetBalance, 07);
  }

  inp(GUSBase+StatusPort);
  outp(GUSBase+CommandPort, DMACtrl);
  inp(GUSBase+DataHiPort);
  outp(GUSBase+CommandPort, SampleCtrl);
  inp(GUSBase+DataHiPort);
  outp(GUSBase+CommandPort, IRQStatus);
  inp(GUSBase+DataHiPort);

  GUSOutByte(Initialize, 7);

  for (i = 0; i < voices; i++)
  {
    outp(GUSBase+ActiveVoicePort, i);
    GUSOutByte(VolRampRate, 0x1F);       // fast ramping
  }

  outp(GUSBase, 0);
}




/*
==============================================================================
GUSProbe

Out:
0: not found
1: found

btw, why there's 0xAA in _EVERY_ source? because guys from renaissance told
about it. what does it mean? ultradox rulez.
btw2, let's become original and use different values :)
==============================================================================
*/
static int GUSProbe(void)
{
  ubyte          temp;

  GUSOutByte(Initialize, 0);
  GUSDelay();
  GUSDelay();
  GUSOutByte(Initialize, 1);

  GUSPoke(0, 0xCC);
  temp = GUSPeek(0);

  GUSOutByte(Initialize, 0);

  if (temp == 0xCC)
    return 1;
  else
    return 0;
}




/*
==============================================================================
GUSFind

Set valid GUSBase, uses GUSProbe function
Out:
0: GUS not found
1: GUS found
==============================================================================
*/
static int GUSFind(void)
{
  for (GUSBase = 0x210; GUSBase < 0x290; GUSBase += 0x10)
  {
    if (GUSProbe())
      break;
  }

  if (GUSBase < 0x290)
    return 1;
  else
    return 0;
}




/*
==============================================================================
GUSCheckEnv

Find GUS environment string (ULTRASND=...)
==============================================================================
*/
static int GUSCheckEnv(void)
{
  char          *Env;

  Env = getenv("ULTRASND");
  if (Env == NULL)
    return 0;
  if (sscanf(Env, "%hx", &GUSBase) != 1)
    return 0;

  return 1;
}




/*
==============================================================================
GUSFindMem

Find the amount of Gravis memory (256 - 1024k)
==============================================================================
*/
static void GUSFindMem(void)
{
  /* trick comes from FMOD by FireLight */
  GUSReset(14);
  outp(GUSBase, M_Output);

  GUSPoke(0x40000, 0x71);
  if (GUSPeek(0x40000) != 0x71)
  {
    GUSMem = 0x3FFFF;
    return;
  }
  GUSPoke(0x80000, 0x33);
  if (GUSPeek(0x80000) != 0x33)
  {
    GUSMem = 0x7FFFF;
    return;
  }
  GUSPoke(0xC0000, 0x11);
  if (GUSPeek(0xC0000) != 0x11)
    GUSMem = 0xBFFFF;
  else
    GUSMem = 0xFFFFF;
}




/*
==============================================================================
GUSSetVolume

VOL = linear volume, index of GUSVol table, range 0-64
BTW, using volume ramping is better than this routine.
==============================================================================
*/
static void GUSSetVolume(ubyte voice, ubyte volume)
{

  outp(GUSBase+ActiveVoicePort, voice);
  outp(GUSBase+CommandPort, SetVol);

  outpw(GUSBase+DataLoPort, GUSVol[volume]);
  GUSDelay();
  outpw(GUSBase+DataLoPort, GUSVol[volume]);
}




/*
==============================================================================
GUSSetBalance

Set the panning of voice
BALANCE: panning value, range: 0 (far left)-128 (middle)-255 (far rite)
==============================================================================
*/
static void GUSSetBalance(ubyte voice, ubyte balance)
{
  outp(GUSBase+ActiveVoicePort, voice);
  GUSOutByte(SetBalance, balance >> 4);         /* convert to 0-15 */
}



/*
==============================================================================
GUSSetFreq

Frequency is converted to internal GUS freq format
Uncomment to get realtime calculated GUS freq (current version: lookup table)
==============================================================================
*/
static void GUSSetFreq(ubyte voice, uword freq)
{
  register uword RealFreq;

  RealFreq = GUSCalcFreq(freq);

  outp(GUSBase+ActiveVoicePort, voice);
  GUSOutWord(SetVoiceFreq, RealFreq);
}



/*
==============================================================================
GUSStopVoice
==============================================================================
*/
static void GUSStopVoice(ubyte voice)
{
  ubyte temp;

  outp(GUSBase+ActiveVoicePort, voice);

  outp(GUSBase+CommandPort, ReadVoiceMode);
  temp = inp(GUSBase+DataHiPort);

  GUSOutByte(WriteVoiceMode, (temp & 0xDF) | (C_VoiceStopped|C_StopVoice));
  GUSDelay();
  GUSOutByte(WriteVoiceMode, (temp & 0xDF) | (C_VoiceStopped|C_StopVoice));
}



/*
==============================================================================
GUSPlayVoice

MODE = GUS play mode (0/8 - no loop/loop)
BEGIN = begin of sample in GUS mem
START = where should we start playing the sample
        (can be also called loop start)
END = guess...
==============================================================================
*/
static void GUSPlayVoice(ubyte voice, ubyte mode, udword begin, udword start, udword end)
{
  GUSStopVoice(voice);

  GUSOutWord(LoopBeginLo, (begin >> 7) & 8191);
  GUSOutWord(LoopBeginHi, (begin & 127) << 9);

  GUSOutWord(LoopStartLo, (start >> 7) & 8191);
  GUSOutWord(LoopStartHi, (start & 127) << 9);

  GUSOutWord(LoopEndLo, (end >> 7) & 8191);
  GUSOutWord(LoopEndHi, (end & 127) << 9);

  GUSOutByte(WriteVoiceMode, mode & ~(C_VoiceStopped|C_StopVoice));
}



/*
==============================================================================
GUSVolRamp

Volume ramping - ramp from old volume to the current one.

Inspired by Technomancer (a.k.a. Dominik Behr) codes, thanx!
==============================================================================
*/
static void GUSVolRamp(ubyte voice, ubyte vol)
{
  uword         OldVol;
  uword         NewVol, temp;
  ubyte         Control;

  outp(GUSBase+ActiveVoicePort, voice);
  outp(GUSBase+CommandPort, ReadVol);
  OldVol = inpw(GUSBase+DataLoPort);

  NewVol = GUSVol[vol];
  if (OldVol == NewVol)
    return;

  GUSOutByte(VolCtrl, V_RampStopped | V_StopRamp);
  GUSDelay();

  if (NewVol > OldVol)
    Control = 0;
  else
  {
    Control = V_Direction;                      /* change direction */
    temp = NewVol;                              /* xchg volumes     */
    NewVol = OldVol;
    OldVol = temp;
  }

  GUSOutWord(VolRampStart, OldVol);
  GUSOutWord(VolRampEnd, NewVol);
  GUSOutByte(VolCtrl, Control);
  GUSDelay();
}