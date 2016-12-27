/*
===============================================================================
        MODULE LOADER v2.01beta

        Copyright (C) 1997, Maciej Sini뭥 (AKA Yarpen/Swirl)

        Support for:
        ) standard Amiga 4 channel modules (M.K.)
        ) FastTracker v1.?? modules (4-32 channels) (xxCHN/xxCH)

        Quite well commented, I hope.
===============================================================================
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>
#include "wicked.h"

//#define MODDEBUG
#define MP              36              // distance from the middle (panning)

typedef struct MODSIGN          /* that's for searching for signature */
{
  char          Sign[4];
  ubyte         NrChans;
  char          Desc[40];
} MODSIGN;

static MODHDR   MHdr;
static MODSIGN  ModSigns[] =
{
 {"M.K.", 4, "ProTracker 4-Channel"}, {"4CHN", 4, "FastTracker 4CHN"},
 {"6CHN", 6, "FastTracker 6CHN"}, {"8CHN", 8, "FastTracker 8CHN"},
 {"10CH", 10, "FastTracker 10CHN"}, {"12CH", 12, "FastTracker 12CHN"},
 {"14CH", 14, "FastTracker 14CHN"}, {"16CH", 16, "FastTracker 16CHN"},
 {"18CH", 18, "FastTracker 18CHN"}, {"20CH", 20, "FastTracker 20CHM"},
 {"22CH", 22, "FastTracker 22CHN"}, {"24CH", 24, "FastTracker 24CHN"},
 {"26CH", 26, "FastTracker 26CHN"}, {"28CH", 28, "FastTracker 28CHN"},
 {"30CH", 30, "FastTracker 30CHN"}, {"32CH", 32, "FastTracker 32CHN"}
};
static ubyte    NrModSigns = sizeof(ModSigns)/sizeof(MODSIGN);

/* original period table from mod-form.txt by some of MOD-giants.
   we have to scan this table and try to find a note for period.
   extended for five octaves by me. */
static uword    pertab[60] =
{
   1712,1616,1525,1440,1357,1281,1209,1141,1077,1017,961,907, //C-0 to B-0
   856,808,762,720,678,640,604,570,538,508,480,453,           //C-1 to B-1
   428,404,381,360,339,320,302,285,269,254,240,226,           //C-2 to B-2
   214,202,190,180,170,160,151,143,135,127,120,113,           //C-3 to B-3
   107,101,95,90,85,80,76,71,67,64,60,57                      //C-4 to B-4
};

uword Fine2C4Spd[16] =
{
  8363, 8413, 8463, 8529, 8581, 8651, 8723, 8757,
  7895, 7941, 7985, 8046, 8107, 8169, 8232, 8280
};


// standard panning positions for MODs.
static ubyte MODPanning[MAX_CHANNELS] = {0x80-MP, 0x80+MP, 0x80+MP, 0x80-MP,
                                         0x80-MP, 0x80+MP, 0x80+MP, 0x80-MP,
                                         0x80-MP, 0x80+MP, 0x80+MP, 0x80-MP,
                                         0x80-MP, 0x80+MP, 0x80+MP, 0x80-MP,
                                         0x80-MP, 0x80+MP, 0x80+MP, 0x80-MP,
                                         0x80-MP, 0x80+MP, 0x80+MP, 0x80-MP,
                                         0x80-MP, 0x80+MP, 0x80+MP, 0x80-MP,
                                         0x80-MP, 0x80+MP, 0x80+MP, 0x80-MP};

static char  MODLoadStr[] =
{"MOD Loader v2.01beta. Copyright (C) Yarpen/Swirl ["__DATE__"]"};
static char  MODLdrType[3] = {"MOD"};

static void ML_LoadMOD(char *mname);
static void ML_Cleanup(void);
static char ML_Identify(char *mname);

/* MODule loader */
LOADER          Load_MOD =
{
  &MODLoadStr,
  &MODLdrType,
  0,
  ML_LoadMOD,
  ML_Identify,
  ML_Cleanup,
  0
};


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�

// exchange bytes (lsb/msb)
uword Amiga(uword);
#pragma aux Amiga =\
        "xchg     al,ah",\
        "add      ax,ax",\
        parm [ax] value [ax];





/*
=================
ML_SafeMalloc
=================
*/
static void *ML_SafeMalloc(udword ile)
{
  void          *temp;

  temp = malloc(ile);
  if (!temp)
    shutUp("Cannot allocate memory! I need %lu bytes!", ile);

  return temp;
}


/*
=================
ML_ShowMODInfo
=================
*/
#ifdef MODDEBUG
static void ML_ShowMODInfo(void)
{
  printf("Name:            %s\n", Mod.WModName);
  printf("Type:            %s\n", Mod.WModType);
  printf("Channels:        %d\n", Mod.NrChans);
  printf("Orders:          %d\n", Mod.NrOrders);
  printf("Patterns:        %d\n", Mod.NrPats);
  printf("Mem used:        %d\n", Mod.MemoryUsed);
  printf("GUSMem used:     %d\n", Mod.GUSMemUsed);
  printf("Sum first ords:  %d, %d, %d, %d, %d\n", Mod.Orders[0], Mod.Orders[1],
    Mod.Orders[2], Mod.Orders[3], Mod.Orders[4]);
}


/*
=====================
ML_ShowMODSamplesInfo
=====================
*/
static void ML_ShowMODSamplesInfo(void)
{
  int           i;

  printf("##|sample name            |length|repeat|repend|vol|ftu|\n");
  printf("--+-----------------------+------+------+------+---+---+\n");
  for (i = 0; i < 31; i++)
  {
    printf("%02X|%-23s|%6d|%6d|%6d|%3d|%3d|\n", i, Mod.Samples[i].Name,
      Mod.Samples[i].Len, Mod.Samples[i].LoopStart,
      Mod.Samples[i].LoopEnd, Mod.Samples[i].Vol,
      Mod.Samples[i].Finetune>7?Mod.Samples[i].Finetune-16:Mod.Samples[i].Finetune);
    if (i && !(i % 20))
    {
      if (!getch())
        getch();
      printf("\n");
    }
  }
}
#endif


/*
=================
ML_VerifyMOD

Check if module contains valid signature
Determine the number of channels
=================
*/
static char ML_VerifyMOD(char *What)
{
  int           i;
  bool          found = 0;

  for (i = 0; i < NrModSigns; i++)
  {
    if (!memcmp(ModSigns[i].Sign, What, 4))
    {
      Mod.NrChans = ModSigns[i].NrChans;
      strcpy(Mod.WModType, ModSigns[i].Desc);
      found = 1;
      break;
    }
  }
  return found;
}



/*
=================
ML_LoadHeader

Load module header, convert informations
=================
*/
static void ML_LoadHeader(FILE *f)
{
  int           i;
  MODSAMPLE      *CurSamp;

  // be careful with this!
  Mod.WModName = (char *)ML_SafeMalloc(23);
  Mod.WModType = (char *)ML_SafeMalloc(40);
  Mod.Flags = 0x0;

  // get memory for samples (one big block)
  // we support only 31-samples MODs
  Mod.Samples = malloc(31 * sizeof(XSAMPLE));
  if (!Mod.Samples)
    shutUp("ML_LoadHeader(): Cannot Allocate Memory For Samples Info!");
  Mod.MemoryUsed = 31 * sizeof(XSAMPLE);

  // read module header from file & check if it's really a module
  SafeFRead(&MHdr, sizeof(MODHDR), f);
  if (!ML_VerifyMOD(&MHdr.MK))
    shutUp("ML_LoadHeader(): Invalid MOD file (signature not found). %s",
    MHdr.MK);

  strncpy(Mod.WModName, MHdr.SongName, 20);
  Mod.WModName[20] = '\0';
  memcpy(Mod.Orders, MHdr.Orders, 128);
  Mod.NrOrders = MHdr.SongLen-1;

  Mod.InitSpeed = 6;
  Mod.InitTempo = 125;

  // convert samples to extended format
  for (i = 0; i < 31; i++)
  {
    CurSamp = &MHdr.Samples[i];

    // get some memory for sample's name, 24 is enough for MOD
    Mod.Samples[i].Name = (char *)ML_SafeMalloc(24);
    Mod.Samples[i].Len = Amiga(CurSamp->Len);
    Mod.Samples[i].LoopStart= Amiga(CurSamp->Repeat);
    Mod.Samples[i].LoopEnd = Amiga(CurSamp->Replen);
    Mod.Samples[i].LoopEnd += Mod.Samples[i].LoopStart;
    Mod.Samples[i].Vol = CurSamp->Vol;

    /* In theory I should make finetune -8 -- 7,
       but it's not necessary, coz my period table goes like:
       0 to 7 and then -8 to -1 and finetune in file is 0 -- 16
    */
    Mod.Samples[i].Finetune = (CurSamp->Finetune & 0xF); /* lower 4 bits */
    Mod.Samples[i].C4Spd = 8363;
    Mod.Samples[i].C4Spd = Fine2C4Spd[CurSamp->Finetune];

    // loop cannot be > length
    if (Mod.Samples[i].LoopEnd > Mod.Samples[i].Len)
      Mod.Samples[i].LoopEnd = Mod.Samples[i].Len;

    // determine mode (loop: 8, no loop: 0)
    if (Amiga(CurSamp->Replen) <= 2)
    {
      Mod.Samples[i].LoopEnd = Mod.Samples[i].LoopStart = 0;
      Mod.Samples[i].Loop = 0;
    }
    else
      Mod.Samples[i].Loop = 8;

    strncpy(Mod.Samples[i].Name, CurSamp->Name, 22);
    Mod.Samples[i].Name[22] = '\0';             // terminate mark
  }
}



/*
=================
ML_LoadPatterns

Load patterns from file and convert 'em to our format

In MOD:
 _____byte 1_____   byte2_    _____byte 3_____   byte4_
/                 /        /                 /
0000          0000-00000000  0000          0000-00000000

Upper four    12 bits for    Lower four    Effect command.
bits of sam-  note period.   bits of sam-
ple number.                  ple number.
=================
*/
static void ML_LoadPatterns(FILE *f)
{
  udword        AllPSize;
  PATT          *CurPat;
  ubyte         temp[4];
  int           i, j, k;
  uword         period, note;

  // change this if necessary
  AllPSize = 64 * sizeof(PATT) * Mod.NrChans;

  // get mem for array of pointers
  Mod.Patterns = malloc((Mod.NrPats+1) * sizeof(PATT *));
  if (!Mod.Patterns)
    shutUp("ML_LoadPatterns: Cannot Allocate Mem For Patterns Pointers!");
  Mod.MemoryUsed += (Mod.NrPats+1) * sizeof(PATT *);

  for (i = 0; i <= Mod.NrPats; i++)
  {
    // get mem for every single pattern
    Mod.Patterns[i] = malloc(AllPSize);
    if (!Mod.Patterns[i])
      shutUp("ML_LoadPatterns: Cannot Allocate Mem For Pattern #%d!", i);

    CurPat = Mod.Patterns[i];
    Mod.MemoryUsed += AllPSize;

    for (j = 0; j < 64; j++)                    // rows
    {
      for (k = 0; k < Mod.NrChans; k++)         // channels
      {
        SafeFRead(&temp, sizeof(temp), f);


//-------------------------------------------------------------------------------
// Based on MikMak's codes, modified for 5 octaves
// Scanning through pertab, searching for valid period, setting the note.
// The current version works with S3M octaves (9) and C4Spd.
        period = (((uword)temp[0] & 0xF) << 8) + temp[1];

        if (period)
        {
          for (note = 24; note < 60+24; note++)
          {
            if (period == pertab[note-24])
              break;
          }
          if (note == 60+24)
            note = 24;
          //note++;
        }
        else
          note = NONOTE;
//-------------------------------------------------------------------------------


        /* sample = 0 --> no sample, use the previous one */
        CurPat->Sample = (temp[0] & 0xF0) + (temp[2] >> 4);
        CurPat->Vol = NOVOL;
        CurPat->Note = note;
        CurPat->Cmd = (temp[2] & 0xF);
        CurPat->CmdParm = temp[3];

        CurPat++;                       // next channel
      }
    }
  }
}




/*
=================
ML_LoadMOD

Main module loading routine
=================
*/
static void ML_LoadMOD(char *mname)
{
  FILE          *f;
  int           i;

  f = SafeFOpen(mname, "rb");

  ML_LoadHeader(f);

  Mod.NrPats = 0;
  for (i = 0; i < 128; i++)
  {
    if (MHdr.Orders[i] > Mod.NrPats)
      Mod.NrPats = MHdr.Orders[i];
  }
  Mod.NrIns = 31;

  ML_LoadPatterns(f);

  Mod.GUSMemUsed = 0;
  for (i = 0; i < 31; i++)
  {
    LDR_LoadSample(i, f, 0);
  }

  memcpy(&Mod.Panning, MODPanning, Mod.NrChans);

  fclose(f);
}




/*
=================
ML_Identify
=================
*/
static char ML_Identify(char *mname)
{
  FILE          *f;
  char          Buf[4];

  f = SafeFOpen(mname, "rb");
  fseek(f, 1080, SEEK_SET);
  SafeFRead(&Buf, 4, f);
  fclose(f);
  return (ML_VerifyMOD(&Buf));
}



/*
=================
ML_Cleanup
=================
*/
static void ML_Cleanup(void)
{
  int           i;

  // free allocated thingies
  for (i = 0; i < Mod.NrIns; i++)
  {
    if (Mod.Samples[i].Name)
      free(Mod.Samples[i].Name);
  }
  if (Mod.Samples)
    free(Mod.Samples);
  for (i = 0; i <= Mod.NrPats; i++)
  {
    if (Mod.Patterns[i])
      free(Mod.Patterns[i]);
  }
  if (Mod.Patterns)
    free(Mod.Patterns);
}