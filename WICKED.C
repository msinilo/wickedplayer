#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>
#include <dos.h>
#include <i86.h>
#include <math.h>
#include "wicked.h"
#include "kbd.h"

//#define DEBUG
//#undef DEBUG

#define         BARLEN  32
#define         WYPUKLY 0
#define         WKLESLY 1

char            *ScreenBuf;
ubyte           BGColor, FGColor;


//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void SetCursor(word xy);
#pragma aux SetCursor =\
        "xor bx,bx",\
        "mov ah,02",\
        "int 10h",\
        parm [dx] modify [ax bx];

byte GetScan(void);
 #pragma aux GetScan =\
         "xor ax,ax",\
         "int 16h",\
         value [ah];

void ClearK(void);
 #pragma aux ClearK =\
         "xor cl,cl",\
         "mov ch,66",\
         "mov ax,05",\
         "int 16h",\
         modify [cx ax];



/*
=================
Set80x50
=================
*/
void Set80x50(void)
{
  union REGS    r;

  vmode(3);

  r.w.ax = 0x1202;
  r.h.bl = 0x30;
  int386(0x10, &r, &r);

  r.w.ax = 0x1112;
  r.h.bl = 0x0;
  int386(0x10, &r, &r);

  r.w.ax = 0x0103;
  r.w.cx = 0x1000;
  int386(0x10, &r, &r);
}

/*
=================
W_SetCols
=================
*/
void W_SetCols(ubyte b, ubyte f)
{
  BGColor = b;
  FGColor = f;
}

/*
=================
SendData
=================
*/
void SendData(udword ile)
{
  memcpy((char *)0xB8000, ScreenBuf, ile);
}


/*
=================
ClearScreen
=================
*/
void ClearScreen(udword ile)
{
  memset(ScreenBuf, 0, ile);
  SendData(ile);
}


/*
=================
SetColor
=================
*/
void SetColor(ubyte color, ubyte r, ubyte g, ubyte b)
{
  outp(0x3C8, color);
  outp(0x3C9, r);
  outp(0x3C9, g);
  outp(0x3C9, b);
}


/*
=================
W_PutChar
=================
*/
void W_PutChar(ubyte x, ubyte y, char what)
{
  uword         pos = (y * 160) + (x << 1);

  ScreenBuf[pos] = what;
  ScreenBuf[pos+1] = (BGColor << 4) + FGColor;
}


/*
=================
W_PutMsg
=================
*/
void W_PutMsg(ubyte x, ubyte y, char *msg)
{
  int           i;

  for (i = 0; *(msg + i) != '\0'; i++)
    W_PutChar(x+i, y, *(msg + i));
}


/*
=================
W_PutNMsg
=================
*/
void W_PutNMsg(ubyte x, ubyte y, char *msg, ubyte len)
{
  ubyte         i;

  for (i = 0; i < len; i++)
    W_PutChar(x+i, y, *(msg + i));
}


/*
=================
W_PutHdr
=================
*/
void W_PutHdr(ubyte x, ubyte y, ubyte x2, char *msg)
{
  ubyte         i;
  ubyte         len = x2-x;

  i = len/2 - (strlen(msg)/2);
  W_PutMsg(i+x, y, msg);
}


/*
=================
W_PutSmallBox
=================
*/
void W_PutSmallBox(ubyte x, ubyte y, char bog, char bof, char mg, char mf, char *msg)
{
  W_SetCols(bog, bof);
  W_PutMsg(x, y, "ş ");
  W_SetCols(mg, mf);
  W_PutMsg(x+2, y, msg);
}


/*
=================
W_PutBigBox
=================
*/
void W_PutBigBox(ubyte x, ubyte y, ubyte x2, ubyte y2, ubyte typ)
{
  int           i;

  if (typ == WKLESLY)
    W_SetCols(0, 8);
  else
    W_SetCols(0, 15);

  W_PutChar(x, y, 'Ú');
  for (i = x+1; i < x2; i++)
    W_PutChar(i, y, 'Ä');
  for (i = y+1; i < y2; i++)
    W_PutChar(x, i, '³');
  W_PutChar(x, y2, 'À');

  if (typ == WKLESLY)
    W_SetCols(0, 15);
  else
    W_SetCols(0, 8);

  W_PutChar(x2, y, '¿');
  for (i = x+1; i < x2; i++)
    W_PutChar(i, y2, 'Ä');
  W_PutChar(x2, y2, 'Ù');
  for (i = y+1; i < y2; i++)
    W_PutChar(x2, i, '³');
}


/*
=================
ShowBar
=================
*/
void ShowBar(ubyte bar, ubyte x, ubyte y)
{
   int          z;

   for (z = 1; z < BARLEN; z++)
   {
     if (z > bar * BARLEN/64)          /* clear it */
     {
       W_SetCols(0, 8);
       W_PutChar(x+z-1, y, '');
     }
     else
     {
       if (z > 40 * BARLEN/64)
         W_SetCols(0, 14);
       if (z > 60 * BARLEN/64)
         W_SetCols(0, 12);
       if (z <= 40 * BARLEN/64)
         W_SetCols(0, 10);

       W_PutChar(x+z-1, y, '');
     }
   }
}



/*
=================
ShowSamples
=================
*/
#ifdef DEBUG
void ShowSamples(void)
{
  int           i;
  char          String[256];
  ubyte         Pos;
  char          NrSpaces, SpaceBuf[256];

  W_SetCols(0, 7);
  Pos = 11+Mod.NrChans+1;
  Pos = 0;

  W_PutMsg(0, Pos, "Nr³Samplename             ³Vol³FineT ³Loop  ³LoopE ³Len   ³Mode");
  W_PutMsg(0, Pos+1, "ÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÅÄÄÄÄÄÄÅÄÄÄÄÄÄÅÄÄÄÄÄÄÅÄÄÄÄÄÄÅÄÄÄÄ");
  for (i = 0; i < 31; i++)
  {
    NrSpaces = 23-strlen(Mod.Samples[i].Name);
    memset(SpaceBuf, ' ', NrSpaces);
    SpaceBuf[NrSpaces] = '\0';
    if (Mod.Samples[i].Len)
      sprintf(String, "%2d³%-28s³%3d³%6d³%6d³%6d³%6d³%2d", i, Mod.Samples[i].Name,
        Mod.Samples[i].Vol, Mod.Samples[i].Finetune, Mod.Samples[i].LoopStart,
        Mod.Samples[i].LoopEnd, Mod.Samples[i].Len, Mod.Samples[i].Loop);
    else
      sprintf(String, "%2d³%-28s³ -------------- unused -------------",
        i, Mod.Samples[i].Name);

    W_PutMsg(0, Pos+2+i, String);
  }
}
#endif


/*
=================
Extension
=================
*/
char *Extension(char *fname)
{
  return(strrchr(fname, '.')+1);
}

//===============================================================================


/*
==============
main
==============
*/
void main(int argc, char **argv)
{
  int           i, z;
  char          String[256];
  char          Notes[12][2] = {"C-", "C#", "D-", "D#", "E-", "F-", "F#",
                                "G-", "G#", "A-", "A#", "B-"};
  char          Note[3];
  char          *VPtr = (char *)0xA0000;
  char          Start;
  uword         Min, Sec;
  char          CurKey;
  char          *Effects;
  char          EfxIndex;
  bool          IsPoland = 0;
  WMOD          *Module;




//-------------------------------------------------------------------------------
// here starts our code
//-------------------------------------------------------------------------------
  argc--;
  argv++;

  printf("%s", WBanner);                /* show da banner */
  if (!argc)
  {
    printf("Usage: WICKED.EXE <module2play> <--- MOD or S3M\n");
    exit(1);
  }


  ScreenBuf = malloc(80*50*2);          /* get mem for screen buffer */
  if (!ScreenBuf)
    shutUp("Not Enough Mem!");
  Effects = malloc(800);
  if (!Effects)
    shutUp("Not Enough Mem!");

  strcpy(Effects, "ArpeggioPorta  Porta  Porta  ");
  strcat(Effects, "Vibrato Port+VlsVib+Vls Tremolo ");
  strcat(Effects, "Panning S-OffsetVolSlidePosJump ");
  strcat(Effects, "SetVol  PatBreakDUPA!DUPSetTempo");
  strcat(Effects, "SetSpeedXFineSlXFineSlFineVib ");
  strcat(Effects, "GlobalV Rtrg+Vsl");
  strcat(Effects, "Filter  FinePrtFinePrtGliss   ");
  strcat(Effects, "VibWave FinetuneLoopPat TrmWave ");
  strcat(Effects, "16PosPanRetrig  FineVolFineVol");
  strcat(Effects, "NoteCut DELAY  DelayPatInvtLoop");
  strcat(Effects, "        ");




  strupr(argv[0]);

  UsedLdr = &Load_S3M;
  while (UsedLdr)
  {
    if (!UsedLdr->Identify(argv[0]))
      UsedLdr= UsedLdr->Next;
    else
      break;
  }

  if (!UsedLdr)
    shutUp("Sorry, This Format Is Not Supported By Any Of Loaders!");

  UsedDrv = &DrvGUS;

  if (!UsedDrv->Init())                 /* init drivers    */
    shutUp("Cannot Initialize Sound Driver!");
  printf("ş %s\n", UsedDrv->Name);
  printf("ş %s\n", UsedLdr->Name);
  printf("%s found at 0x%X with %dk of memory.\n",
    UsedDrv->DeviceName, UsedDrv->DeviceBase, (UsedDrv->DeviceMem >> 10)+1);

  /* load module */
  printf("Loading module. Please wait...\n");
  UsedLdr->Load(argv[0]);
  addShutUp(UsedLdr->Cleanup);

  /* set initial values */
  MP_SetGlobalVol(64);
  MP_SetStereo(0);
  IsPoland = LDR_Search4Poland();

  /* set video mode and colors */
  Set80x50();
  ClearScreen(8000);
  addShutUp(vmode3);

  SetColor(0, 45, 45, 45);
  SetColor(15, 63, 63, 63);
  ClearScreen(8000);

  /* show our copyrite plate */
  W_PutBigBox(0, 0, 79, 49, WYPUKLY);
  W_PutBigBox(3, 1, 79-3, 4, WYPUKLY);
  W_SetCols(0, 15);
  W_PutHdr(3, 2, 79-3, "Wicked Player V1.20beta");
  W_PutHdr(3, 3, 79-3, "Copyright (C) 1996-97 by Maciej Sini’o");

  /* draw module info plate  */
  W_PutBigBox(3, 5+33, 43, 5+33+10, WKLESLY);
  /* send module info        */
  W_SetCols(0, 15);
  W_PutMsg(5, 6+33, Mod.WModName);
  W_SetCols(0, 12);
  if (IsPoland)
    W_PutMsg(43-7, 5+33, "POLAND!");
  W_SetCols(0, 14);
  W_PutMsg(5, 7+33,  "Module Type   : ");
  W_PutMsg(5, 8+33,  "Channels      : ");
  W_PutMsg(5, 9+33,  "Order         : ");
  W_PutMsg(5, 10+33, "Row           : ");
  W_PutMsg(5, 11+33, "Pattern       : ");
  W_PutMsg(5, 12+33, "Speed/Tempo   : ");
  W_PutMsg(5, 13+33, "Time          : ");
  W_SetCols(0, 15);
  W_PutMsg(21, 7+33, Mod.WModType);
  sprintf(String, "%d", Mod.NrChans);
  W_PutMsg(21, 8+33, String);
  W_SetCols(0, 12);
  W_PutMsg(5, 14+33, "I like Inertia's design :)");


  /* draw technical plate */
  W_PutBigBox(45, 5+33, 79-3, 5+33+10, WKLESLY);
  W_SetCols(0, 15);
  sprintf(String, "%s", UsedDrv->DeviceName);
  W_PutMsg(47, 6+33, String);
  W_SetCols(0, 14);
  W_PutMsg(47, 7+33, "Base Port     : ");
  W_PutMsg(47, 8+33, "DRAM Memory   : ");
  W_PutMsg(47, 9+33, "Mixing speed  : ");
  W_PutMsg(47,10+33, "GUS Mem Used  : ");
  W_PutMsg(47,11+33, "Hi Mem Used   : ");
  W_PutMsg(47,12+33, "Global Volume : ");
  W_SetCols(0, 15);
  sprintf(String, "0%Xh", UsedDrv->DeviceBase);
  W_PutMsg(63, 7+33, String);
  sprintf(String, "%dk", (UsedDrv->DeviceMem >> 10)+1);
  W_PutMsg(63, 8+33, String);
  sprintf(String, "%dkHz", GUSMixingSpeed+1);
  W_PutMsg(63, 9+33, String);
  sprintf(String, "%dk", (Mod.GUSMemUsed >> 10)+1);
  W_PutMsg(63, 10+33, String);
  sprintf(String, "%dk", (Mod.MemoryUsed >> 10)+1);
  W_PutMsg(63, 11+33, String);

  /* draw channel plate */
  W_PutBigBox(1, 5, 79-1, 5+32, WKLESLY);

  MP_PlayMod();


//-------------------------------------------------------------------------------
// here we have so called interphace
//-------------------------------------------------------------------------------
  do
  {
    if (inp(0x60) == 0x3B)
      Ord++;

    /*CurKey = GetScan();
    switch(CurKey)
    {
      case KBD_LEFT: if (Ord)
                       Ord--;
                     break;
      case KBD_RITE: Ord++;
                     if (Ord >= Mod.NrOrders)
                     {
                       Ord = Row = 0;
                       MP_SetClock(0);
                     }
                     break;
    } */

    Min = MP_GetClock()/60;
    Sec = MP_GetClock()%60;

    W_SetCols(0, 15);
    sprintf(String, "%d/%d  ", Ord, Mod.NrOrders);
    W_PutMsg(21, 9+33, String);
    sprintf(String, "%d/63 ", Row);
    W_PutMsg(21, 10+33, String);
    sprintf(String, "%d/%d  ", Mod.Orders[Ord], Mod.NrPats);
    W_PutMsg(21, 11+33, String);
    sprintf(String, "%d/%d  ", Speed, BPM);
    W_PutMsg(21, 12+33, String);
    sprintf(String, "%02d:%02d", Min, Sec);
    W_PutMsg(21, 13+33, String);
    sprintf(String, "%d", MP_GetGlobalVol());
    W_PutMsg(63, 12+33, String);

    for (i = 0; i < Mod.NrChans; i++)
    {
      if (Tracks[i].Play && (Tracks[i].Note != NONOTE && Tracks[i].Note != KEYOFF))
        Tracks[i].Bar = Tracks[i].Vol;
      if (Tracks[i].Bar > 0)
        Tracks[i].Bar--;

      switch(Tracks[i].Cmd)
      {
        case 0xC:
        case 0x5:
        case 0x6:
        case 0x9:
        case 0xA: Tracks[i].Bar = Tracks[i].Vol;
                  break;
        case 0xE: switch(Tracks[i].CmdParm >> 4)
                  {
                    case 0xA:
                    case 0xB: Tracks[i].Bar += (Tracks[i].Vol-Tracks[i].Bar);
                              break;
                    default:  break;
                  }
        default:  break;
      }


      /* prepare note to show */
      memcpy(Note, Notes[Tracks[i].Note % 12], 2);
      Note[2] = '\0';

      /* prepare all to show  */
      sprintf(String, "%2s%1d", Note, Tracks[i].Note/12);
      W_SetCols(0, 15);
      if (Tracks[i].Play)
        W_PutMsg(2, 6+i, String);
      sprintf(String, "%2d", Tracks[i].Vol);
      W_SetCols(0, 14);
      W_PutMsg(2+4, 6+i, String);
#ifndef DEBUG
      sprintf(String, "%-28s", Mod.Samples[Tracks[i].Smp].Name);
#else
      sprintf(String, "%5d %2X", Tracks[i].Period, Tracks[i].Pan);
#endif
      W_SetCols(0, 11);
#ifndef DEBUG
      if (Tracks[i].Play)
#endif
        W_PutMsg(2+4+3, 6+i, String);
      ShowBar(Tracks[i].Bar, 2+4+3+28, 6+i);

      EfxIndex = Tracks[i].Cmd;

      if (EfxIndex == 0xE)
        EfxIndex = 0x16+(Tracks[i].CmdParm >> 4);
      else
      if (!EfxIndex && !Tracks[i].CmdParm)
        EfxIndex = 38;
      if (EfxIndex > 38)
        EfxIndex = 38;

      memcpy(String, Effects+(EfxIndex*8), 8);
      String[8] = '\0';


      W_SetCols(0, 14);
      W_PutMsg(2+4+28+3+BARLEN, 6+i, String);
    }

    /* send it to da screen */
    waitvbl();
    SendData(8000);

  } while(inp(0x60) != 1);
//-------------------------------------------------------------------------------
// here we have the end of so called interphace
//-------------------------------------------------------------------------------



  MP_StopMod();
  UsedLdr->Cleanup();
  vmode(3);
  printf("--- please DO NOT spread it away. this is only beta version ---\n");
}