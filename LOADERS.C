#include <stdio.h>
#include <stdlib.h>
#include <types.h>
#include "wicked.h"

#define NR_LOADERS 2            /* MOD, S3M */

WMOD            Mod;
udword          GUSPos = 0;

/* funny feature: recognizing the most known polish composers */
static char     *PolishComposers[] =
{"XTD", "FALCON", "KEYG", "BARTESEK", "ROBO", "RAIDEN", "SCORPIK",
 "LSD", "DAN", "CIELIK", "DAVE", "SNOOPY", "NORVEG"
};
static ubyte    NrPLComposers = sizeof(PolishComposers)/sizeof(char *);

LOADER          *UsedLdr;

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�


/* does exactly what the name says                                        */
/* BTW, i'm 99% sure that there's built-in C function which does the same */
/* what this routine, but i dunno its name :) you know, i've learnt C     */
/* from other people's codes, not from boox, so i don't know all nice     */
/* intructions. ok, let's stop it, coz otherwise this comment will be     */
/* longer then the function itself. hmm, in fact it already is...         */
static char ChrUpr(char ch)
{
  if (ch >= 'a' && ch <= 'z')
    ch -= ('a' - 'A');
  return ch;
}



/*
=================
Ldr_SafeMalloc
=================
*/
static void *Ldr_SafeMalloc(udword ile)
{
  void          *temp;

  temp = malloc(ile);
  if (!temp)
    shutUp("Cannot allocate memory! I need %lu bytes!", ile);

  return temp;
}




/*
=====================
LDR_Search4Poland
=====================
*/
bool LDR_Search4Poland(void)
{
  int           i, j, z, x;
  bool          found = 0;

  /* first, let's check some famous Polish trademarks */
  if (!memcmp(Mod.WModName, "##", 2))                    /* XTD           */
    return 1;
  if (Mod.WModName[strlen(Mod.WModName)-1] == '_')       /* Falcon        */
    return 1;
  if (Mod.WModName[0] == '>')                            /* KeyG (new)    */
    return 1;
  if (!memcmp(Mod.WModName, "<<<", 3))                   /* KeyG (old)    */
    return 1;
  if (!memcmp(&Mod.WModName[strlen(Mod.WModName)-4], "RDN!", 4))/* Raiden */
    return 1;
  if (!memcmp(Mod.WModName, "++", 2))                    /* Robo          */
    return 1;

  /* i am not sure why this works, but hey, don't look a gift horse
     in the mouth, rite? */
  for (i = 0; i < NrPLComposers; i++)
  {
    for (j = 0; j < 4; j++)
    {
      z = 0;

       /* let's search for the first char */
      while (z < strlen(Mod.Samples[j].Name))
      {
        if (ChrUpr(Mod.Samples[j].Name[z]) == PolishComposers[i][0])
        {
          x = 1;
          while (ChrUpr(Mod.Samples[j].Name[x+z]) == PolishComposers[i][x] &&
            x < strlen(Mod.Samples[j].Name) && x < strlen(PolishComposers[i]))
            x++;
          if (x == strlen(PolishComposers[i]))
          {
            found = 1;
            break;
          }
          else
            z++;
        }
        else
          z++;
      }

      if (found)
        break;
    }
    if (found)
      break;
  }
  return found;
}


/*
=================
LDR_LoadSample

Load samples from file and kick 'em to GUS memory
=================
*/
void LDR_LoadSample(int i, FILE *f, ubyte type)
{
  udword        Len;
  char          *Buf;

  Len = Mod.Samples[i].Len;

  if (Len)
  {
    GUSPos += 31;
    GUSPos &= -32;                      /* GUS address tricks */
    if (GUSPos + Len > UsedDrv->DeviceMem)
      shutUp("ML_LoadSample(): Not Enough GUS Mem! GUSPos: %d, Sample Len: %d!",
        GUSPos, Len);

    Buf = (char *)Ldr_SafeMalloc(Len+1);
    SafeFRead(Buf, Len, f);

    GUSDumpSample(GUSPos, Buf, Len, type);
    free(Buf);

    Mod.GUSMemUsed += Len;
  }

  // trick against the nasty GUS clicks
  if (Mod.Samples[i].Loop)
  {
    /* if loop than copy the first byte in the loop
       one place beyond the end of the loop */
    GUSPoke(GUSPos + Mod.Samples[i].LoopEnd, GUSPeek(GUSPos + Mod.Samples[i].LoopStart));
  }
  else
  {
    /* no loop: zero the byte beyond the end of sample */
    GUSPoke(GUSPos + Len, 0);
  }

  Mod.Samples[i].GUSPos = GUSPos;
  if (Len)
    GUSPos += (Len+1);
}