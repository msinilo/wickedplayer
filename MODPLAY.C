/*
===============================================================================
    MODULE PLAYING ROUTINES

    Copyright (C) 1997, Maciej Sini뭥 (AKA Yarpen/Swirl)

    Quite well commented, maybe someone will understand something.
===============================================================================
*/


#include <stdlib.h>
#include <dos.h>
#include <stdio.h>
#include <types.h>
#include "wicked.h"

#define OCTAVES         9

TRACK           Tracks[MAX_CHANNELS];
word            Ord, Row;
ubyte           Speed;
uword           BPM;
static PATT     *CurPat;
static uword    PatDelay = 0;
static bool     ReverseStereo = 0;
static ubyte    GlobalVol = 64;
static udword   Timer0 = 0, Timer1 = 0;

// these cool "volatiles" comes from Extravaganza modplayer, thanx.
// but whadda hell does it mean?
static volatile unsigned short BigTick;
static volatile unsigned short Tick1;
static volatile unsigned short Tick2;
static volatile unsigned short TimerRate;
static volatile uword          Tick;

static uword MiddleOctave[12] = {1712, 1616, 1524, 1440, 1356, 1280,
                          1208, 1140, 1076, 1016, 960, 907};

/* periods for 9 octaves. middle octave (4) from TECH.DOC, the rest
   is runtime generated */
static uword Periods[12*OCTAVES];

// sine table (0-pi)
static word VibTable[32] =
{
        0,24,49,74,97,120,141,161,
        180,197,212,224,235,244,250,253,
        255,253,250,244,235,224,212,197,
        180,161,141,120,97,74,49,24
};



//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�

// here lies old timer IRQ
static void (__interrupt __far *OldTimer)(void);

static void MP_SetTimer(uword BPM);
void MP_SetClock(udword t);

/* returns pointer to da current pattern position */
static PATT *GetCurPat(void)
{
  return (Mod.Patterns[Mod.Orders[Ord]] + (Row * Mod.NrChans));
}



/*
=================
CalcAllOctaves

Calc all octaves
=================
*/
static void CalcAllOctaves(void)
{
  int           Oct, Note;

  for (Note = 0; Note < 12; Note++)
    Periods[0*12+Note] = MiddleOctave[Note]*16;

  for (Oct = 1; Oct < 9; Oct++)
    for (Note = 0; Note < 12; Note++)
      Periods[Oct*12+Note] = Periods[(Oct-1)*12+Note]/2;
}



/*
=================
DoVolSlide

process volume slide effect
!It could be called also on tick0 (if fast slides are used).
In such a case the volume shouldn't be set!
=================
*/
static void DoVolSlide(int i, TRACK *t)
{
  ubyte         ChangeX, ChangeY;

  ChangeX = t->CmdParm >> 4;
  ChangeY = t->CmdParm & 0xF;

  if (ChangeX)
  {
    t->Vol += ChangeX;
    if (t->Vol > 64)
      t->Vol = 64;
  }
  else
  {
    t->Vol -= ChangeY;
    if (t->Vol < 0)
      t->Vol = 0;
  }

  UsedDrv->SetVolume(i, t->Vol*GlobalVol/64);
}



/*
=================
DoTonePorta
=================
*/
static void DoTonePorta(int i, TRACK *t)
{
  if (t->Period == t->PortaPeriod)
    return;

  if (t->Period > t->PortaPeriod)
  {
    t->Period -= (t->PortaSpd << 2);      /* here might be an overflow! */
    if ((word)t->Period < t->PortaPeriod) /* so, here must be signed    */
      t->Period = t->PortaPeriod;
  }
  else
  {
    t->Period += (t->PortaSpd << 2);
    if (t->Period > t->PortaPeriod)
      t->Period = t->PortaPeriod;
  }

  UsedDrv->SetFreq(i, t->Period);
}



/*
=================
DoVibrato

rite = 0 for fine vibrato
rite = 2 for normal vibrato
=================
*/
static void DoVibrato(int i, TRACK *t, char rite)
{
  word          Tmp;
  word          TPeriod;

  Tmp = VibTable[t->VibPos & 0x1F];
  Tmp *= t->VibDepth;
  Tmp >>= (7-rite);
  if (t->VibPos >= 0)
    TPeriod = t->Period + Tmp;
  else
    TPeriod = t->Period - Tmp;

  if (TPeriod > MAX_PERIOD)
    TPeriod = MAX_PERIOD;
  if (TPeriod < MIN_PERIOD)
    TPeriod = MIN_PERIOD;

  UsedDrv->SetFreq(i, TPeriod);

  t->VibPos += t->VibSpd;

  if (t->VibPos > 31)
    t->VibPos -= 64;
}


/*
=================
DoTremolo
=================
*/
static void DoTremolo(int i, TRACK *t)
{
  word          Tmp;

  Tmp = VibTable[(t->TremoloPos & 0x1F)];
  Tmp *= t->TremoloDepth;
  Tmp >>= 6;
  if (t->TremoloPos < 0)
    Tmp = t->Vol - Tmp;
  else
    Tmp += t->Vol;

  if (Tmp < 0)
    Tmp = 0;
  else
  if (Tmp > 64)
    Tmp = 64;

  UsedDrv->SetVolume(i, Tmp*GlobalVol/64);
  t->TremoloPos += t->TremoloSpd;
  if (t->TremoloPos > 31)
    t->TremoloPos -= 64;
}






/*
=================
UpdateNote
=================
*/
static void UpdateNote(int i, PATT *PPos, TRACK *t)
{
  ubyte         s, n, v;                        /* sample, note, volume    */
  ubyte         cmd, cmdp, cmx, cmy;            /* cmd, parm, parmx, parmy */
  bool          PJump = 0, PBreak = 0;

  /* make some init stuff, update track structure */
  t->Cmd = PPos->Cmd;
  t->CmdParm = PPos->CmdParm;
  s = PPos->Sample;
  n = PPos->Note;
  v = PPos->Vol;
  cmd = PPos->Cmd;
  cmdp = PPos->CmdParm;
  cmx = cmdp >> 4;
  cmy = cmdp & 0xF;

  t->OldCmd = t->Cmd;
  if (t->CmdParm)                               /* only if a parameter was */
    t->OldCmdParm = t->CmdParm;                 /* given!                  */

  if (s)                                        /* sample 0: no sample */
  {
    t->Smp = s-1;                               /* here 0 is allowed   */
    t->Vol = Mod.Samples[s-1].Vol;              /* copy sample's vol   */
    t->C4Spd = Mod.Samples[s-1].C4Spd;          /* and C4Spd           */
  }
  s = t->Smp;                                   /* if no sample, then get */
                                                /* the previous one       */
  if (Mod.Samples[s].Loop)
  {
    t->GUSBegin = Mod.Samples[s].GUSPos;
    t->GUSLoopBegin = Mod.Samples[s].LoopStart + Mod.Samples[s].GUSPos;
    t->GUSEnd = Mod.Samples[s].LoopEnd + Mod.Samples[s].GUSPos;
    t->Loop = 8;
    t->SampleOffset = 0;
  }
  else
  {
    t->GUSBegin = Mod.Samples[s].GUSPos;
    t->GUSLoopBegin = Mod.Samples[s].GUSPos;
    t->GUSEnd = Mod.Samples[s].Len + Mod.Samples[s].GUSPos;
    t->Loop = 0;
    t->SampleOffset = 0;
  }

  if (n != NONOTE && n != KEYOFF)               /* was a note given?      */
  {
    t->Note = n;
    if (cmd != 0x3 && cmd != 0x5)               /* if not porta2note then */
      t->Period = 8363*Periods[n]/t->C4Spd;     /* let's set the period   */
    else                                        /* otherwise...           */
      t->PortaPeriod = 8363*Periods[n]/t->C4Spd;/* only porta period      */

    t->VibPos = 0;
    t->ArpPos = 0;
    t->Play = 1;
  }
  else
    t->Play = 0;

  if (v != NOVOL && v <= 0x40)
    t->Vol = v;
  if (n == KEYOFF)
    t->Vol = 0;



//-------------------------------------------------------------------------------
// let's process some tick 0 effects (processed only one time per row)
// all checks (volume & period) will be done before playing the sample!
//-------------------------------------------------------------------------------
  switch(cmd)
  {
    case 0x0:   if (cmdp)
                {
                  t->ArpeggioVals[0] = t->Note;
                  t->ArpeggioVals[1] = t->Note+cmx;
                  t->ArpeggioVals[2] = t->Note+cmy;
                }
                break;

    case 0x1:   if (!cmdp)                    /* Porta Up                  */
                  cmdp = t->OldCmdParm;       /* fill with old value       */
                break;

    case 0x2:   if (!cmdp)                    /* Porta Down                */
                  cmdp = t->OldCmdParm;       /* fill with old value       */
                break;

    case 0x3:   if (cmdp)                     /* Tone Portamento           */
                  t->PortaSpd = cmdp;         /* also called Porta To Note */
                /* porta period -- was calculated earlier!                 */
                t->Play = 0;                  /* don't play it now!        */
                break;

    case 0x4:                                 /* Vibrato                   */
    case 0x13:  if (cmy)                      /* Fine Vibrato              */
                  t->VibDepth = cmy;
                if (cmx)
                  t->VibSpd = cmx;
                break;


    case 0x5:   t->Play = 0;                  /* Tone Porta+VolSlide       */
                if (!cmdp)
                  cmdp = t->OldCmdParm;
                break;                        /* uses Efx03 values         */

    case 0x6:   if (!cmdp)
                  cmdp = t->OldCmdParm;       /* Vibrato+VolSlide          */

    case 0x7:   if (cmy)                      /* Tremolo                   */
                  t->TremoloDepth = cmy;
                if (cmx)
                  t->TremoloSpd = cmx;
                break;

    case 0x8:   t->Pan = cmdp;                /* DMP Panning (not PT efx)  */
                break;                        /* 0 (left) - 0xFF (rite)    */

    case 0x9:   if (cmdp)                     /* Sample Offset             */
                  t->SampleOffset = (cmdp << 8);
                if (t->SampleOffset > Mod.Samples[t->Smp].Len)
                  t->SampleOffset = Mod.Samples[t->Smp].Len;
                break;

    case 0xA:   if (!cmdp)                    /* VolSlide                  */
                  cmdp = t->OldCmdParm;
                if ((Mod.Flags & MF_FASTVSLIDES))   /* processed on tick 0 */
                {                                   /* only if fast slides */
                  if ((cmdp & 0xF0))                /* are used [S3M]      */
                  {
                    t->Vol += (cmdp >> 4);
                    if (t->Vol > 64)
                      t->Vol = 64;
                  }
                  else
                  {
                    t->Vol -= (cmdp & 0xF);
                    if (t->Vol < 0)
                      t->Vol = 0;
                  }
                }
                break;

    case 0xB:   Ord = cmdp;                   /* Jump To Pattern           */
                Row = -1;     /* it will be incremented after UpdateNote() */
                if (Ord > Mod.NrOrders)
                {
                  Ord = 0;
                  MP_SetClock(0);
                }
                PJump = 0;
                break;

    case 0xC:   t->Vol = cmdp;                /* Set Volume (I love it! :) */
                break;

    case 0xD:   Row = (cmx * 10) + cmy - 1;   /* Pattern Break             */
                if (Row > 62)
                  Row = -1;
                if (!PBreak && !PJump)/*^^^^ it will be increased after!   */
                  Ord++;              /* if there's is a jump then do nuffin */
                if (Ord > Mod.NrOrders)
                {
                  Ord = 0;
                  MP_SetClock(0);
                }
                PBreak = 1;
                break;

    case 0xE:   switch(cmx)                   /* Extended PT Effects       */
                {
                                              /* Fine Porta Up             */
                  case 0x1: if (!cmy)
                            {
                              cmdp = t->OldCmdParm;
                              cmy = cmdp & 0xF;
                            }
                            t->Period -= (cmy << 2);
                            break;
                                              /* Fine Porta Down           */
                  case 0x2: if (!cmy)
                            {
                              cmdp = t->OldCmdParm;
                              cmy = cmdp & 0xF;
                            }
                            t->Period += (cmy << 2);
                            break;

                                              /* Set Finetune              */
                  case 0x5: Mod.Samples[t->Smp].Finetune = (cmy & 0xF);
                            Mod.Samples[t->Smp].C4Spd = Fine2C4Spd[cmy & 0xF];
                            t->C4Spd = Fine2C4Spd[cmy & 0xF];
                            t->Period = 8363*Periods[t->Note]/t->C4Spd;
                            break;

                            /* probably this should be used in other way   */
                            /* more like sliding, but who cares about      */
                            /* no used effects?                            */
                  case 0x8: t->Pan = (cmy << 4);
                            break;

                  case 0xA: if (!cmy)
                            {
                              cmdp = t->OldCmdParm;
                              cmy = (cmdp & 0xF);
                            }
                            t->Vol += cmy;    /* Fine Vol Slide Up         */
                            break;

                  case 0xB: if (!cmy)
                            {
                              cmdp = t->OldCmdParm;
                              cmy = (cmdp & 0xF);
                            }
                            t->Vol -= cmy;    /* Fine Vol Slide Down       */
                            break;

                  case 0xE: PatDelay = cmy;   /* Pattern Delay             */
                            break;

                  default:  break;
                }
                break;

    case 0xF:   if (cmdp <= 0x20)             /* Set Speed                 */
                {
                  if (!cmdp)
                    Speed = 1;
                  else
                    Speed = cmdp;
                }
                else
                {
                  BPM = cmdp;
                  MP_SetTimer(BPM);
                }
                break;


/*************************** S3M NATIVE EFFECTS ****************************/
    case 0x10:  if (!cmdp)                    /* S3M Set Speed             */
                  Speed = 1;
                else
                  Speed = cmdp;
                break;

    case 0x11:  if (!cmdp)                    /* Extra Fine Slide Up       */
                   cmdp = t->OldCmdParm;
                t->Period -= cmdp;
                break;

    case 0x12:  if (!cmdp)                    /* Extra Fine Slide Down     */
                  cmdp = t->OldCmdParm;
                t->Period += cmdp;
                break;

    case 0x14:  GlobalVol = cmdp;
                if (GlobalVol > 64)
                  GlobalVol = 64;
                break;

    case 0x15:  if (!cmdp)
                  cmdp = t->OldCmdParm;
                break;

    default:    break;
  }


  t->CmdParm = cmdp;            /* update paremeter  */


//-------------------------------------------------------------------------------
// now let's kick da sample to our cool GUS
//-------------------------------------------------------------------------------
  if (t->Vol > 64)
    t->Vol = 64;
  else
  if (t->Vol < 0)
    t->Vol = 0;
  if (t->Period > MAX_PERIOD)
    t->Period = MAX_PERIOD;
  if (t->Period < MIN_PERIOD)
    t->Period = MIN_PERIOD;

  UsedDrv->SetVolume(i, t->Vol*GlobalVol/64);
  UsedDrv->SetFreq(i, t->Period);
  UsedDrv->SetPanning(i, t->Pan);
  if (t->Play)                                  /* play the sample    */
  {
    UsedDrv->PlayVoice(i, t->Loop, t->GUSBegin + t->SampleOffset,
      t->GUSLoopBegin, t->GUSEnd);
  }
}


/*
=================
UpdateEffects
=================
*/
static void UpdateEffects(int i, TRACK *t)
{
  ubyte         cmd, cmdp, cmx, cmy;
  ubyte         n;
  word          temp;

  cmd = t->Cmd;
  cmdp = t->CmdParm;
  cmx = cmdp >> 4;
  cmy = cmdp & 0xF;
  n = t->Note;



//-------------------------------------------------------------------------------
// Effects processed every clock tick.
// Volume & Period must be checked every time.
//-------------------------------------------------------------------------------
  switch(cmd)
  {
    case 0x0:   if (cmdp > 0)                 /* Arpeggio                  */
                {
                  UsedDrv->SetFreq(i, Periods[t->ArpeggioVals[t->ArpPos]]);
                  t->ArpPos++;
                  if (t->ArpPos == 3)
                    t->ArpPos = 0;
                }
                break;

    case 0x1:   t->Period -= (cmdp << 2);     /* Porta Slide Up            */
                if (t->Period < MIN_PERIOD)
                  t->Period = MIN_PERIOD;
                UsedDrv->SetFreq(i, t->Period);
                break;

    case 0x2:   t->Period += (cmdp << 2);     /* Porta Slide Down          */
                if (t->Period > MAX_PERIOD)
                  t->Period = MAX_PERIOD;
                UsedDrv->SetFreq(i, t->Period);
                break;

    case 0x3:   DoTonePorta(i, t);            /* Tone Portamento           */
                break;

    case 0x4:   DoVibrato(i, t, 2);           /* Vibrato                   */
                break;

    case 0x5:   DoTonePorta(i, t);            /* Tone Porta+VolSlide       */
                DoVolSlide(i, t);
                break;

    case 0x6:   DoVibrato(i, t, 2);           /* Vibrato+VolSlide          */
                DoVolSlide(i, t);
                break;

    case 0x7:   DoTremolo(i, t);              /* Tremolo                   */
                break;

    case 0xA:   DoVolSlide(i, t);             /* Volume Slide              */
                break;

    case 0xE:   switch(cmx)                   /* Extended PT Effects       */
                {
                  /* Retrigger Note
                     Restart note at tick number y */
                  case 0x9: if (!cmy)
                              break;
                            if ((Tick % cmy) == 0)
                              UsedDrv->PlayVoice(i, t->Loop,
                                t->GUSBegin + t->SampleOffset,
                                t->GUSLoopBegin, t->GUSEnd);
                            break;

                  /* Cut Note
                     Set vol to 0 (cut note off) at tick number y
                     That's what StarScream said in his MultiTracker manual */
                  case 0xC: if (Tick == cmy)
                            {
                              t->Vol = 0;
                              UsedDrv->SetVolume(i, t->Vol);
                            }
                            break;

                  case 0xD:

                  default:  break;
                }
                break;

    case 0x13:  DoVibrato(i, t, 0);
                break;

    case 0x15:  if (!cmy)
                  break;
                if ((Tick % cmy) == 0)
                {
                  if (cmx)
                  {
                    switch(cmx)
                    {
                      case 0x1: t->Vol--;
                                break;
                      case 0x2: t->Vol -= 2;
                                break;
                      case 0x3: t->Vol -= 4;
                                break;
                      case 0x4: t->Vol -= 8;
                                break;
                      case 0x5: t->Vol -= 16;
                                break;
                      case 0x6: t->Vol *= (2/3);
                                break;
                      case 0x7: t->Vol >>= 1;
                                break;
                      case 0x9: t->Vol++;
                                break;
                      case 0xA: t->Vol += 2;
                                break;
                      case 0xB: t->Vol += 4;
                                break;
                      case 0xC: t->Vol += 8;
                                break;
                      case 0xD: t->Vol += 16;
                                break;
                      case 0xE: t->Vol *= (3/2);
                                break;
                      case 0xF: t->Vol += t->Vol;
                                break;
                      default:  break;
                    }
                    if (t->Vol > 64)
                      t->Vol = 64;
                    if (t->Vol < 0)
                      t->Vol = 0;
                    UsedDrv->SetVolume(i, t->Vol*GlobalVol/64);
                  }
                  UsedDrv->PlayVoice(i, t->Loop,
                    t->GUSBegin + t->SampleOffset,
                    t->GUSLoopBegin, t->GUSEnd);
                }
                break;

    default:    break;
  }
}



/*
=================
MP_ModHandler

haha, the best commented routine ever, don't you think?
=================
*/
void __interrupt __far MP_ModHandler(void)
{
  int           i;

  Tick++;
  Timer0++;
  if ( Timer0 == (BPM*2/5) )
  {
    Timer1++;
    Timer0 = 0;
  }


  /* check if we should process next row   */
  if (Tick >= Speed)
  {
    CurPat = GetCurPat();
    if (!PatDelay)                              /* Pattern Delay?       */
    {
      for (i = 0; i < Mod.NrChans; i++)
        UpdateNote(i, CurPat++, &Tracks[i]);    /* update current row   */

      Row++;                                    /* increase row         */
      if (Row == 64)                            /* change this with XM! */
      {
        Row = 0;
        Ord++;                                  /* next order           */

        if (Ord > Mod.NrOrders)                 /* end of song?         */
        {
          Ord = 0;                              /* let's restart!       */
          MP_SetClock(0);
        }
      }
      Tick = 0;                                 /* wait next xx ticks   */
    }
    else
    {
      Tick = 0;                                 /* wait next xx ticks   */
      PatDelay--;
    }
  }
  else
  {
    for (i = 0; i < Mod.NrChans; i++)
      UpdateEffects(i, &Tracks[i]);
  }

  /* is it time to call our old interrupt? */
  Tick2 = Tick1;
  Tick1 += TimerRate;
  if (Tick2 > Tick1)                            /* overflow?   */
    _chain_intr(OldTimer);                      /* let's call old IRQ   */
  else                                          /* no overflow */
    outp(0x20, 0x20);                           /* send only EOI signal */
}


/*
=================
MP_SetTimer

Set timer speed (BPM*2/5) (TimerRate)
=================
*/
static void MP_SetTimer(uword BPM)
{
  uword        temp;
  uword        hz;

  if (BPM == 0)
    temp = 0;
  else
  {
    hz = (BPM << 1) / 5;
    temp = 0x1234DD / hz;
  }
  TimerRate = temp;          /* set timer rate to "quite" big value */
                                /* the tempo won't be as big never!    */

  outp(0x43, 0x36);
  outp(0x40, temp & 0xFF);
  outp(0x40, (ubyte)((temp >> 8) & 0xFF));
}



/*
=================
MP_PlayMod

Prepare GUS, set timer and start playing
=================
*/
void MP_PlayMod(void)
{
  DRIVER        *Drv;
  int           i;

  Drv = UsedDrv;
  Ord = 0;
  Row = 0;
  PatDelay = 0;

  CalcAllOctaves();

  Drv->Reset(Mod.NrChans);
  memset(&Tracks, 0, sizeof(Tracks));

  for (i = 0; i < Mod.NrChans; i++)
  {
    /* our panning goes 0 (far left) - 128 (middle) - 255 (far rite) */
    if (!ReverseStereo)
      Tracks[i].Pan = Mod.Panning[i];
    else
      Tracks[i].Pan = 0xFF - Mod.Panning[i];

    Drv->SetVolume(i, 0);
    Drv->SetFreq(i, 856);
    Drv->SetPanning(i, Tracks[i].Pan);
    Drv->PlayVoice(i, 0, 0, 0, 0);
    Drv->StopVoice(i);
  }

  Speed = Mod.InitSpeed;                        /* standard settings */
  Tick = Mod.InitSpeed-1;
  BPM = Mod.InitTempo;

  OldTimer = _dos_getvect(0x8);
  _dos_setvect(0x8, MP_ModHandler);

  MP_SetTimer(BPM);
}



/*
=================
MP_StopMod

Stop playing, reset timer
=================
*/
void MP_StopMod(void)
{
  int           i;

  _dos_setvect(0x8, OldTimer);
  MP_SetTimer(0);

  for (i = 0; i < Mod.NrChans; i++)
    UsedDrv->StopVoice(i);
}



/*
=================
MP_SetGlobalVol
=================
*/
void MP_SetGlobalVol(byte vol)
{
  if (vol < 0)
    vol = 0;
  if (vol > 64)
    vol = 64;
  GlobalVol = vol;
}


/*
=================
MP_GetGlobalVol
=================
*/
ubyte MP_GetGlobalVol(void)
{
  return GlobalVol;
}

/*
=================
MP_SetStereo
=================
*/
void MP_SetStereo(bool stereo)
{
  ReverseStereo = stereo;
}



/*
=================
MP_GetClock
=================
*/
udword MP_GetClock(void)
{
  return Timer1;
}


/*
=================
MP_SetClock
=================
*/
void MP_SetClock(udword t)
{
  Timer1 = t;
  Timer0 = 0;
}