/*
===============================================================================
    S3M Loader v0.7beta
    Copyright (C) 1997, Maciej Sini’o (AKA Yarpen/Swirl)
    All rights reserved.

    Some informations about S3M file format:
      - up to 99 instruments
      - 9 octaves
      - 32 channels
      - up to 255 patterns
      - up to 255 orders
      - creator: Sami Tammilehto a.k.a. Psi/Future Crew
===============================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>
#include "wicked.h"

//#define S3MDEBUG

#define MP              36              // distance from the middle (panning)

static S3MHDR          S3MHdr;
static word            InsParaPtrs[100];
static word            PatsParaPtrs[256];
static char            S3MRemap[MAX_CHANNELS];

// standard panning positions for S3Ms.
static ubyte S3MPanning[MAX_CHANNELS] = {0x80-MP, 0x80+MP, 0x80-MP, 0x80+MP,
                                         0x80-MP, 0x80+MP, 0x80-MP, 0x80+MP,
                                         0x80-MP, 0x80+MP, 0x80-MP, 0x80+MP,
                                         0x80-MP, 0x80+MP, 0x80-MP, 0x80+MP,
                                         0x80-MP, 0x80+MP, 0x80-MP, 0x80+MP,
                                         0x80-MP, 0x80+MP, 0x80-MP, 0x80+MP,
                                         0x80-MP, 0x80+MP, 0x80-MP, 0x80+MP,
                                         0x80-MP, 0x80+MP, 0x80-MP, 0x80+MP};

static void SL_LoadS3M(char *sname);
static char SL_Identify(char *sname);
static void SL_Cleanup(void);

static char S3MLdrName[] =
{"S3M Loader v0.8beta. Copyright (C) Yarpen/Swirl ["__DATE__"]"};
static char S3MLdrType[3] = {"S3M"};

/* MODule loader */
LOADER          Load_S3M =
{
  &S3MLdrName,
  &S3MLdrType,
  0,
  SL_LoadS3M,
  SL_Identify,
  SL_Cleanup,
  &Load_MOD
};



//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ





/*
===============================================================================
SL_SafeMalloc

SafeMalloc routine for .S3M loader
===============================================================================
*/
static void *SL_SafeMalloc(udword ile)
{
  void          *temp;

  temp = malloc(ile);
  if (!temp)
    shutUp("Cannot allocate memory! I need %lu bytes!", ile);

  return temp;
}




/*
===============================================================================
SL_LoadHeader

Alloc memory for basic structures, read header, convert header information
===============================================================================
*/
static void SL_LoadHeader(FILE *f)
{
  /* be careful with this! */
  Mod.WModName = (char *)SL_SafeMalloc(32);
  Mod.WModType = (char *)SL_SafeMalloc(40);

  /* read header & verify it */
  SafeFRead(&S3MHdr, sizeof(S3MHDR), f);
  if (memcmp(S3MHdr.SCRM, "SCRM", 4))
    shutUp("SL_LoadHeader(): Invalid S3M file (SCRM signature not found)!");

  /* copy header information */
  Mod.NrOrders = S3MHdr.SongLen;
  Mod.NrPats = S3MHdr.NrPats;
  Mod.NrIns = S3MHdr.NrIns;
  Mod.InitVol = S3MHdr.GlobalVol;
  Mod.InitSpeed = S3MHdr.InitSpeed;
  if (S3MHdr.InitTempo > 0x20)
    Mod.InitTempo = S3MHdr.InitTempo;
  else
    Mod.InitTempo = 0x7F;
  Mod.Stereo = S3MHdr.MasterVol >> 7;

  Mod.Flags = 0x0;
  if ((S3MHdr.Flags & 64) || (S3MHdr.CWTV == 0x1300))
    Mod.Flags |= MF_FASTVSLIDES;


  /* copy module name */
  strncpy(Mod.WModName, S3MHdr.SongName, 28);
  Mod.WModName[28] = '\0';


  /* prepare module type */
  strcpy(Mod.WModType, "ST");
  switch(S3MHdr.CWTV)
  {
    case 0x1300: strcat(Mod.WModType, "3.00 ");
                 break;
    case 0x1301: strcat(Mod.WModType, "3.01 ");
                 break;
    case 0x1303: strcat(Mod.WModType, "3.03 ");
                 break;
    case 0x1320: strcat(Mod.WModType, "3.20 ");
                 break;
  }
}




/*
===============================================================================
SL_CountChannels

Determine the number of channels, prepare S3MRemap table, init panning
===============================================================================
*/
static void SL_CountChannels(void)
{
  int           i;
  char          CurChan;
  char          NrChansTxt[2];

  /* fill remap table        */
  memset(&S3MRemap, 0xFF, sizeof(S3MRemap));

  /* init number of channels */
  Mod.NrChans = 0;


  /* count number of channels (still buggy!), set initial panning */
  for (i = 0; i < MAX_CHANNELS; i++)
  {
    CurChan = S3MHdr.ChannelSet[i];

    /* bit 8: channel enabled */
    if (!(CurChan & 128))
    {
      S3MRemap[i] = Mod.NrChans;

      if (CurChan & 0x8)
        Mod.Panning[Mod.NrChans] = (0x80+MP);
      else
        Mod.Panning[Mod.NrChans] = (0x80-MP);

      Mod.NrChans++;
    }
  }

  /* add number of channels to module type */
  itoa(Mod.NrChans, NrChansTxt, 10);
  strcat(Mod.WModType, NrChansTxt);
  strcat(Mod.WModType, "-Channel");
}




/*
===============================================================================
SL_LoadOrders

Load orders & parapointers. Calc real number of orders & patterns, convert
orders (kick out markers & end of tune = empty patterns)
===============================================================================
*/
static void SL_LoadOrders(FILE *f)
{
  int           i, j;

  /* read orders & para pointerz */
  SafeFRead(&Mod.Orders, Mod.NrOrders, f);
  SafeFRead(&InsParaPtrs, Mod.NrIns << 1, f);
  SafeFRead(&PatsParaPtrs, Mod.NrPats << 1, f);

  j = 0;                                        /* real number of orderz  */
  Mod.NrPats = 0;
  for (i = 0; i < Mod.NrOrders; i++)
  {
    if (Mod.Orders[i] < 254)                    /* used???                */
    {
      Mod.Orders[j] = Mod.Orders[i];
      j++;                                      /* yep, so let's increase */
      if (Mod.Orders[i] >= Mod.NrPats)
        Mod.NrPats = Mod.Orders[i];
    }
  }
  Mod.NrOrders = j-1;
}




/*
===============================================================================
SL_LoadIns

Load samples from file (using parapointers). Convert 'em to my own, internal
format which is supported by universal player routine.
After that - dump samples to GUS memory.
===============================================================================
*/
static void SL_LoadIns(FILE *f)
{
  int           i;
  S3MSAMPLE     Sample; /* buffer */

  Mod.Samples = (XSAMPLE *)SL_SafeMalloc(Mod.NrIns * sizeof(XSAMPLE));
  Mod.MemoryUsed = Mod.NrIns * sizeof(XSAMPLE);

  for (i = 0; i < Mod.NrIns; i++)
  {
    fseek(f, InsParaPtrs[i] << 4, SEEK_SET);
    SafeFRead(&Sample, sizeof(Sample), f);

    Mod.Samples[i].Name = (char *)SL_SafeMalloc(29);
    Mod.Samples[i].Len = Sample.Len;
    Mod.Samples[i].LoopStart = Sample.Repeat;
    Mod.Samples[i].LoopEnd = Sample.RepEnd;
    Mod.Samples[i].Vol = Sample.Vol;
    Mod.Samples[i].C4Spd = Sample.C2Spd;
    if (Sample.RepEnd > Sample.Len)
      Mod.Samples[i].LoopEnd = Mod.Samples[i].Len;

    /* loop?? */
    if ((Sample.Flags & 0x1) && Sample.RepEnd > 2)
      Mod.Samples[i].Loop = 8;
    else
    {
      Mod.Samples[i].Loop = 0;
      Mod.Samples[i].LoopStart = 0;
      Mod.Samples[i].LoopEnd = 0;
    }

    strncpy(Mod.Samples[i].Name, Sample.SmpName, sizeof(Sample.SmpName));
    Mod.Samples[i].Name[28] = '\0';           /* termination mark */

    fseek(f, (udword)(Sample.MemSeg << 4), SEEK_SET);
    LDR_LoadSample(i, f, 128);
  }
}



#ifdef S3MDEBUG
/*
===============================================================================
SL_PrintPattern

Debug routine -- it helps me checking if pattern is decoded OK.
===============================================================================
*/
static void SL_PrintPattern(void)
{
  int           i, j;
  char          Notes[12][2] =
{
 "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"
};
  PATT          *CurPat;
  char          Note[3];

  vmode(3);
  printf("ORD: %d\n", Mod.Orders[0]);
  getch();
  for (i = 0; i < 20; i++)
  {
    printf("%02d³", i);
    for (j = 0; j < Mod.NrChans; j++)
    {
      CurPat = Mod.Patterns[Mod.Orders[0]] + (i * Mod.NrChans) + j;
      memcpy(Note, Notes[CurPat->Note%12], 2);
      Note[2] = '\0';

      if (CurPat->Note == NONOTE)
        printf("---");
      else if (CurPat->Note == KEYOFF)
        printf("^^^");
      else
        printf("%2s%1d", Note, CurPat->Note/12);
      if (CurPat->Sample)
        printf("%02d", CurPat->Sample);
      else
        printf("--");
      if (CurPat->Cmd != NOCMD)
        printf("%X%02X³", CurPat->Cmd, CurPat->CmdParm);
      else
        printf("---³");
    }
    printf("\n");
  }
}
#endif



/*
===============================================================================
SL_LoadPats

Load and depack all used patterns.
===============================================================================
*/
static void SL_LoadPats(FILE *f)
{
  int           Pat, Row, i;
  udword        AllPSize = 64 * sizeof(PATT) * Mod.NrChans;
  char          Tmp8, Tmp82;
  word          Tmp16;
  char          Chn;
  PATT          *CurPat;

  /* get memory for pointers to patterns                */
  /* remember that patterns go like:                    */
  /* 0 -- number_of_patterns, not                       */
  /* 0 -- number_of_patterns-1 as you could think       */
  Mod.Patterns = malloc((Mod.NrPats+1) * sizeof(PATT *));
  if (!Mod.Patterns)
    shutUp("SL_LoadPats: Cannot Allocate Mem For Pattern Pointers!");
  Mod.MemoryUsed += (Mod.NrPats+1) * sizeof(PATT *);


//-------------------------------------------------------------------------------
// LET'S GO!
//-------------------------------------------------------------------------------
  for (Pat = 0; Pat <= Mod.NrPats; Pat++)
  {
    fseek(f, PatsParaPtrs[Pat] << 4, SEEK_SET); /* seek at valid position */
    SafeFRead(&Tmp16, sizeof(Tmp16), f);        /* read size of pattern   */

    Mod.Patterns[Pat] = malloc(AllPSize);       /* allocate next pattern  */
    if (!Mod.Patterns[Pat])
      shutUp("SL_LoadPats: Cannot Allocate Mem For Pattern #%d!", i);
    Mod.MemoryUsed += AllPSize;

    /* now, let's clear da pattern, fill it with impossible/invalid values */
    for (Row = 0; Row < 64; Row++)
    {
      for (Chn = 0; Chn < Mod.NrChans; Chn++)
      {
        CurPat = Mod.Patterns[Pat] + (Row * Mod.NrChans) + Chn;
        CurPat->Note = NONOTE;
        CurPat->Sample = 0;
        CurPat->Vol = NOVOL;
        CurPat->Cmd = NOCMD;
        CurPat->CmdParm = 0;
      }
    }


//-------------------------------------------------------------------------------
// main loop for every pattern
//-------------------------------------------------------------------------------
    Row = 0;                                    /* row = 0                */
    while (Row < 64)
    {
      SafeFRead(&Tmp8, sizeof(Tmp8), f);        /* get byte               */
      if (Tmp8 > 0)
      {
        Chn = S3MRemap[Tmp8 & 31];              /* we got the channel!    */

        /* first, let's check if the channel is valid! */
        if (Chn == 0xFF || Chn >= Mod.NrChans)
        {
          /* channel is invalid, let's move da pointer! */
          if ((Tmp8 & 32) > 0)
            fseek(f, 2, SEEK_CUR);
          if ((Tmp8 & 64) > 0)
            fseek(f, 1, SEEK_CUR);
          if ((Tmp8 & 128) > 0)
            fseek(f, 2, SEEK_CUR);
        }
        else
        {
          /* channel is OK, let's read da pattern data  */
          CurPat = Mod.Patterns[Pat] + (Row * Mod.NrChans) + Chn;

/**** Is Note/Sample Given? ****/
          if ((Tmp8 & 32) > 0)
          {
            SafeFRead(&Tmp82, sizeof(Tmp82), f);
            if (Tmp82 == 255)
              CurPat->Note = NONOTE;
            else
            if (Tmp82 == 254)
              CurPat->Note = KEYOFF;
            else
              CurPat->Note = ((Tmp82 >> 4) * 12) + (Tmp82 & 0xF);
            /* octave*12+note */
            SafeFRead(&CurPat->Sample, sizeof(CurPat->Sample), f);
          }

/**** Is Volume Given ****/
          if ((Tmp8 & 64) > 0)
            SafeFRead(&CurPat->Vol, sizeof(CurPat->Vol), f);

/**** Is Cmd/CmdParm Given? ****/
          if ((Tmp8 & 128) > 0)
          {
            SafeFRead(&CurPat->Cmd, sizeof(CurPat->Cmd), f);
            SafeFRead(&CurPat->CmdParm, sizeof(CurPat->CmdParm), f);
          }
        }
      }
      else
        Row++;                          /* end of row */
    }
  }
}




/*
===============================================================================
SL_ConvertEffects

Convert S3M effects to effects supported by the player (usually to ordinary
MOD effects)
Ha, this section is probably better than even in MiDAS!
===============================================================================
*/
static void SL_ConvertEffects(void)
{
  PATT          *CurPat;
  int           Row, Chan, Pat;
  ubyte         cmdp;
  ubyte         OldCmdParms[MAX_CHANNELS];

  memset(&OldCmdParms, 0, sizeof(OldCmdParms));

  for (Pat = 0; Pat <= Mod.NrPats; Pat++)
    for (Row = 0; Row < 64; Row++)
      for (Chan = 0; Chan < Mod.NrChans; Chan++)
      {
        CurPat = Mod.Patterns[Pat] + (Row * Mod.NrChans) + Chan;

        /* it's possible that cmdparm == 0 with some effects */
        if (!CurPat->CmdParm && CurPat->Cmd != 0x3 && CurPat->Cmd != 0x2)
          CurPat->CmdParm = OldCmdParms[Chan];
        else
          OldCmdParms[Chan] = CurPat->CmdParm;

        switch(CurPat->Cmd)
        {
          case 0x0:     CurPat->Cmd = NOCMD;            /* nuffin */
                        CurPat->CmdParm = 0;
                        break;

          case 0x1:     CurPat->Cmd = 0x10;             /* S3M set speed */
                        break;

          case 0x2:     CurPat->Cmd = 0xB;              /* jump          */
                        break;

          case 0x3:     CurPat->Cmd = 0xD;              /* patbreak      */
                        break;

/* D0y: slide down by y                         */
/* Dx0: slide up by x                           */
/* DFy: fine vol down by y                      */
/* DxF: fine vol up by x                        */
          case 0x4:     if ((CurPat->CmdParm >> 4) == 0)
                        {
                          CurPat->Cmd = 0xA;
                        }
                        else
                        if ((CurPat->CmdParm & 0xF) == 0)
                        {
                          CurPat->Cmd = 0xA;
                        }
                        else
                        if ((CurPat->CmdParm & 0xF) == 0xF)
                        {
                          CurPat->Cmd = 0xE;
                          CurPat->CmdParm >>= 4;
                          CurPat->CmdParm &= 0xF;
                          CurPat->CmdParm |= 0xA0;
                        }
                        else
                        {
                          CurPat->Cmd = 0xE;
                          CurPat->CmdParm &= 0xF;
                          CurPat->CmdParm |= 0xB0;
                        }
                        break;

/* Exx: slide down by xx                        */
/* EFx: fine slide down by x                    */
/* EEx: xtra fine slide down by x               */
          case 0x5:     if (CurPat->CmdParm <= 0xDF)
                        {
                          CurPat->Cmd = 0x2;              /* slide down      */
                        }
                        else
                        if (CurPat->CmdParm <= 0xEF)      /* extra fine      */
                        {
                          CurPat->Cmd = 0x12;
                          CurPat->CmdParm &= 0xF;       /* only y          */
                        }
                        else
                        {
                          CurPat->Cmd = 0xE;
                          CurPat->CmdParm &= 0xF;
                          CurPat->CmdParm |= 0x20;    /* fine slide      */
                        }
                        break;

          case 0x6:     if (CurPat->CmdParm <= 0xDF)
                        {
                          CurPat->Cmd = 0x1;            /* slide up        */
                        }
                        else
                        if (CurPat->CmdParm <= 0xEF)
                        {
                          CurPat->Cmd = 0x11;
                          CurPat->CmdParm &= 0xF;       /* only y          */
                        }
                        else
                        {
                          CurPat->Cmd = 0xE;
                          CurPat->CmdParm &= 0xF;
                          CurPat->CmdParm |= 0x10;      /* fine slide      */
                        }
                        break;

          case 0x7:     CurPat->Cmd = 0x3;              /* porta2note    */
                        break;

          case 0x8:     CurPat->Cmd = 0x4;              /* vibrato       */
                        break;

          case 0xA:     CurPat->Cmd = 0x0;              /* arpeggio      */
                        break;

          case 0xB:     CurPat->Cmd = 0x6;              /* vib+vslide    */
                        break;

          case 0xC:     CurPat->Cmd = 0x5;              /* porta+vslide */
                        break;

          case 0xF:     CurPat->Cmd = 0x9;              /* sample offset */
                        break;

          case 0x11:    CurPat->Cmd = 0x15;             /* retrig+vslide */
                        break;

          case 0x12:    CurPat->Cmd = 0x7;              /* tremolo       */
                        break;

          case 0x13:    CurPat->Cmd = 0xE;
                        switch(CurPat->CmdParm >> 4)
                        {
                          case 0x0: break;
                          case 0x2: CurPat->CmdParm = (CurPat->CmdParm&0xF)|0x50;
                                    break;
                          case 0xA: CurPat->Cmd = 0x13;
                                    CurPat->CmdParm &= 0xF;
                                    break;
                          case 0x8: break;
                          default:  CurPat->Cmd = NOCMD;
                                    CurPat->CmdParm = 0;
                                    break;
                        }
                        break;

          case 0x14:    CurPat->Cmd = 0xF;              /* set tempo     */
                        break;

          case 0x15:    CurPat->Cmd = 0x13;             /* fine vibrato  */
                        break;

          case 0x16:    CurPat->Cmd = 0x14;             /* set global vol */
                        break;

          default:      CurPat->Cmd = NOCMD;            /* nuffin */
                        CurPat->CmdParm = 0;
                        break;
        }
      }
}






/*
===============================================================================
SL_LoadS3M

Main S3M loading routine.
===============================================================================
*/
static void SL_LoadS3M(char *sname)
{
  FILE          *f;
  int           i;

  memset(&Mod, 0, sizeof(Mod));
  f = SafeFOpen(sname, "rb");
  SL_LoadHeader(f);
  SL_CountChannels();
  SL_LoadOrders(f);

  if (S3MHdr.DefPan == 252)
  {
    SafeFRead(&Mod.Panning, 32, f);
    for (i = 0; i < MAX_CHANNELS; i++)
    {
      Mod.Panning[i] &= 0xF;
      Mod.Panning[i] <<= 4;
    }
  }
 // else
 //   memcpy(&Mod.Panning, &S3MPanning, MAX_CHANNELS);

  if (!Mod.Stereo)
    for (i = 0; i < MAX_CHANNELS; i++)
      Mod.Panning[i] = 0x80;

  SL_LoadIns(f);
  SL_LoadPats(f);
  fclose(f);

#ifdef S3MDEBUG
  SL_PrintPattern();
#endif
  SL_ConvertEffects();
}




/*
=================
SL_Identify
=================
*/
static char SL_Identify(char *sname)
{
  FILE          *f;
  char          Buf[4];

  f = SafeFOpen(sname, "rb");
  fseek(f, 0x2C, SEEK_SET);
  SafeFRead(&Buf, 4, f);
  fclose(f);

  return !(memcmp(Buf, "SCRM", 4));
}



/*
=================
SL_Cleanup
=================
*/
static void SL_Cleanup(void)
{
  int           i;

  /* free allocated thingys */
  for (i = 0; i < Mod.NrIns; i++)
    if (Mod.Samples[i].Name)
      free(Mod.Samples[i].Name);

  if (Mod.Samples)
    free(Mod.Samples);

  for (i = 0; i <= Mod.NrPats; i++)
    if (Mod.Patterns[i])
      free(Mod.Patterns[i]);

  if (Mod.Patterns)
    free(Mod.Patterns);
}