/*
    Wicked Player Structures.
    Copyright (C) 1996-97, Maciej Siniío a.k.a. Yarpen/Swirl
    Do not care about stupid names as I do not care.
*/


#ifndef WICKED_H
#define WICKED_H

#include <types.h>

#define WBanner \
"-----------------------------------------------------------------------------\n" \
" Wicked Player v1.20beta ["__DATE__"]. Copyright (C) 1996-97, Maciej Siniío  \n" \
"-----------------------------------------------------------------------------\n"

#define MAX_SAMPLES 100                 // should be enough for now
#define MAX_CHANNELS 32                 // wanna get more?
#define MAX_ORDERS 256
#define MAX_PATTERNS 256
#define NONOTE 255                      // max note with 9 octaves is 'bout 120
#define NOVOL 255
#define KEYOFF 254
#define NOCMD 255

#ifndef AMIGA_LIMITS
  #define MAX_PERIOD 27392              // C-0
  #define MIN_PERIOD 48                 // B-8
#else
  #define MAX_PERIOD 1712               // C-3
  #define MIN_PERIOD 56                 // B-5
  /* of course MAX_PERIOD/MIN_PERIOD would change if we used extended */
  /* MOD octaves (5 instead of 3), but that's another story           */
#endif


/* song flags */
#define MF_FASTVSLIDES 0x1

/*
STRUCTURES
----------
*/

typedef struct MODSAMPLE                         // original MOD sample struc
{
  char          Name[22];
  uword         Len;
  byte          Finetune;
  ubyte         Vol;
  uword         Repeat;
  uword         Replen;
} MODSAMPLE;


typedef struct S3MSAMPLE                        // original S3M sample structure
{
  char          Type;                           /* 1 - sample, rest - adlib */
  char          DOSName[12];                    /* 12345678.ABC             */
  char          Krap0;
  uword         MemSeg;                         /* ptr to sample in file    */
  dword         Len;
  dword         Repeat;
  dword         RepEnd;
  char          Vol;
  char          Krap1;
  char          Pack;                           /* 1 - DP30ADPCM            */
  char          Flags;                          /* loops/16/8bits etc.      */
  word          C2Spd;                          /* C2 = C4 :)               */
  word          HiC2Spd;                        /* unused                   */
  char          Krap2[12];
  char          SmpName[28];
  char          SCRS[4];                        /* SCRS signature           */
} S3MSAMPLE;



typedef struct XSAMPLE                          // extended sample structure
{
  char          *Name;                          /* dynamic               */
  udword        Len;                            /* >64k? what for?       */
  udword        LoopStart;                      /* loop start            */
  udword        LoopEnd;                        /* loop end              */
  uword         C4Spd;                          /* C4 (middle C) speed   */
  ubyte         Vol;                            /* volume (0-64)         */
  byte          Finetune;                       /* MOD finetune          */
  ubyte         Loop;                           /* 0 - no loop, 8 - loop */
  udword        GUSPos;                         /* position in GUS mem   */
} XSAMPLE;


typedef struct PATT                             // pattern struc
{                                               /* one line of one chan  */
  ubyte         Sample;
  ubyte         Note;
  ubyte         Cmd;
  ubyte         CmdParm;
  ubyte         Vol;
} PATT;


typedef struct TRACK
{
  bool          Play;                   /* play or not?                   */
  ubyte         Smp;                    /* sample played on this track    */

  udword        GUSBegin;               /* begin of sample in GUS mem     */
  udword        GUSEnd;                 /* end of sample/loop in GUS mem  */
  udword        GUSLoopBegin;           /* begin of loop in GUS mem       */
  uword         SampleOffset;           /* effect number 9                */
  ubyte         Loop;                   /* loop mode (ready to send: 8/0) */

  byte          OldVol;                 /* a kind of backup               */
  byte          Vol;                    /* track volume (0-64)            */
  byte          Finetune;               /* sample finetune (0-15)         */
  uword         C4Spd;                  /* middle C speed                 */
  ubyte         OldCmd;                 /* a kind of backup               */
  ubyte         OldCmdParm;             /* ------||--------               */
  ubyte         Cmd;                    /* track effect                   */
  ubyte         CmdParm;                /* effect parameter               */
  ubyte         Note;                   /* in fact: octave*12+note        */
  ubyte         Pan;                    /* track panning (0-255)          */
  uword         Period;                 /* track period                   */

  ubyte         ArpeggioVals[3];        /* 3 values of arpeggio           */
  char          ArpPos;                 /* current arpeggio pos (0-2)     */
  uword         PortaSpd;               /* portamento speed               */
  uword         PortaPeriod;            /* portamento val                 */
  byte          VibPos;                 /* vibrato position               */
  ubyte         VibDepth;               /* vibrato depth                  */
  ubyte         VibSpd;                 /* vibrato speed                  */
  byte          TremoloPos;             /* as with vib (pos, dph, spd)    */
  ubyte         TremoloDepth;
  ubyte         TremoloSpd;

  ubyte         Bar;                    /* bar value                      */
} TRACK;



typedef struct MODHDR                   // original MOD header structure
{
  char          SongName[20];
  MODSAMPLE     Samples[31];
  ubyte         SongLen;
  ubyte         Restart;                /* not used                     */
  ubyte         Orders[128];
  char          MK[4];
} MODHDR;

typedef struct S3MHDR                   // original S3M header structure
{
  char          SongName[28];
  char          Fucked1A;               /* TYPE terminator              */
  char          FileType;               /* should be 16, I guess        */
  word          Krap0;
  word          SongLen;                /* in orders                    */
  word          NrIns;                  /* number of instruments        */
  word          NrPats;                 /* number of patterns           */
  word          Flags;                  /* vibrato/slide/tempo/etc.     */
  word          CWTV;                   /* version of the tracker?      */
  word          SignFlag;               /* 1 = signed, 2 = unsigned smp */
  char          SCRM[4];                /* should be SCRM signature     */
  char          GlobalVol;              /* default global volume        */
  char          InitSpeed;
  char          InitTempo;              /* = BPM                        */
  char          MasterVol;              /* lower 7 bits - vol (sb) s/m  */
  char          Ultraclick;             /* nr of free-click channels    */
  char          DefPan;                 /* default panning              */
  char          Krap1[8];
  word          Special;                /* ptr to special custom data   */
  char          ChannelSet[32];         /* bit 8: enabled, 0-7: type    */
} S3MHDR;


typedef struct WMOD                     // WickedMOD structure
{
  char          *WModName;              // dynamic
  char          *WModType;              /* comment about the module     */

  ubyte         NrChans;                /* number of channels           */
  uword         NrOrders;               /* number of orders (songlen)   */
  uword         NrPats;                 /* number of patterns           */
  uword         NrIns;                  /* number of samples            */
  ubyte         Orders[MAX_ORDERS];     /* all orders                   */
  char          InitVol;                /* initial values               */
  char          InitTempo;              /* -----||-------               */
  char          InitSpeed;              /* -----||-------               */
  bool          Stereo;                 /* mono/stereo                  */
  char          Panning[MAX_CHANNELS];  /* panning position4every chan  */
  char          Flags;                  /* vslides etc.                 */
  XSAMPLE       *Samples;               /* pointer to samples           */
  PATT          **Patterns;             /* array of pointers to patts   */
  udword        MemoryUsed;             /* memory used by patterns      */
  udword        GUSMemUsed;             /* GUS mem used by samples      */
} WMOD;


/* The credit for this great idea goes to MikMak. Thanks! */
/* Hehe, anyway, for the present it works only with GUS   */
typedef struct DRIVER
{
  char          *Name;
  char          *DeviceName;
  char          DeviceType;             /* 0 = wavetable             */
  uword         DeviceBase;
  udword        DeviceMem;              /* only fÅr wavetable cards! */
  bool          (*Init)(void);
  void          (*Reset)(ubyte voices);
  void          (*DumpSample)(udword loc, char *buffer, uword buflength, ubyte xorval);
  void          (*PlayVoice)(ubyte voice, ubyte mode, udword begin, udword start, udword end);
  void          (*StopVoice)(ubyte voice);
  void          (*SetVolume)(ubyte voice, ubyte volume);
  void          (*SetPanning)(ubyte voice, ubyte balance);
  void          (*SetFreq)(ubyte voice, uword period);
  void          (*PlayStart)(void);
  void          (*PlayStop)(void);
} DRIVER;

/* This one is also the idea of MikMak. Thanks!
   BTW, I have only MIKMOD.H file, I have no idea which version of player
   it is etc. If someone has complete package of MikMod, please send it
   to me... */
typedef struct LOADER
{
  char          *Name;
  char          *Type;                          /* "MOD", "S3M" etc.    */
  bool          (*Init)(void);
  void          (*Load)(char *mname);
  char          (*Identify)(char *mname);       /* check type of module */
  void          (*Cleanup)(void);
  struct LOADER *Next;
} LOADER;


/*
PROTOTYPES
----------
*/
#include "shutup.h"
#include "files.h"


/*
OBJECTS
-------
*/

/* LOADERS.C */
extern LOADER          *UsedLdr;
extern WMOD            Mod;
extern udword          GUSPos;
void   LDR_LoadSample(int i, FILE *f, ubyte type);
bool   LDR_Search4Poland(void);

/* GUS.C */
extern DRIVER          DrvGUS;
extern DRIVER          *UsedDrv;


/* MODLOAD.C */
extern LOADER          Load_MOD;
extern uword           Fine2C4Spd[16];

/* S3MLOAD.C */
extern LOADER          Load_S3M;

/* GUS.C */
extern uword           GUSBase;
extern udword          GUSMem;
extern uword           GUSMixingSpeed;

/* MODPLAY.C */
extern TRACK           Tracks[MAX_CHANNELS];
extern word            Ord, Row;
extern ubyte           Speed;
extern uword           BPM;

/*************************************************************************/
/* Start playing the loaded module                                       */
/*************************************************************************/
void   MP_PlayMod(void);

/*************************************************************************/
/* Stop playing module, unhook timer IRQ, shutup GUS                     */
/*************************************************************************/
void   MP_StopMod(void);

/*************************************************************************/
/* Global Volume: 0-64                                                   */
/*************************************************************************/
void   MP_SetGlobalVol(byte vol);
ubyte  MP_GetGlobalVol(void);

/*************************************************************************/
/* Stereo: 1 - reversed, 0 - normal                                      */
/*************************************************************************/
void   MP_SetStereo(bool stereo);

/*************************************************************************/
/* Clock: can be easily transformed to min:sec                           */
/* Min = MP_GetClock() / 60                                              */
/* Sec = MP_GetClock() % 60                                              */
/*************************************************************************/
udword MP_GetClock(void);
void   MP_SetClock(udword t);


#endif