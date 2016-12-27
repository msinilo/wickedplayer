/* ===============================================================================
      shutUp library v3.01

      copyright (c) 1997, maciej sini’o a.k.a. yarpen

      universal exit, handles pre-exit routines like stop playing,
      dehook irq etc.

      btw, i know that shutDown is more accurate name, but i'm not going
      to change "up" to "down" in all my codes using this library.
   ===============================================================================
*/

#include <i86.h>
#include <stdarg.h>
#include <types.h>

#define MAX_SHUTUPS 8                   /* yeah, yeah, i know - it should   */
                                        /* be dynamic, but this way is      */
                                        /* _A_LOT_ easier (and more stable, */
                                        /* i think).

//°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
/* useful routine, it sets our cool 0x3 video mode */
void vmode3 (void)
{
  union REGS r;
  r.w.ax = 0x003;
  int386 (0x10, &r, &r);
}


//°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
/* it does nothing. probably it's not even necessary, but who carez¨ */
void dummyRt (void)
{
  return;
}


//°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
static struct {                                /* shutUp structure   */
    ubyte ControlByte;
    void (*func)(void);
} shutUpTab[MAX_SHUTUPS] = {
         {FALSE, dummyRt},              /* could be 0 instead of dummyRt */
         {FALSE, dummyRt},
         {FALSE, dummyRt},
         {FALSE, dummyRt},
         {FALSE, dummyRt},
         {FALSE, dummyRt},
         {FALSE, dummyRt},
         {FALSE, dummyRt},
};
static int ShutUpNo = 0;                /* counter of shutUps */

//°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
/* it just adds a routine, which will be called before exiting to DOS */
void addShutUp(void (*funkc)(void))
{
  int i;

  if (ShutUpNo == MAX_SHUTUPS)               /* check if there's free slot */
  {
    for (i = 0; i < ShutUpNo; i++)           /* call all prepared routines */
      if (shutUpTab[i].ControlByte == TRUE)
        shutUpTab[i].func();
    vmode(3);
    printf("Too Many shutUps (There Could Be Up To %d)! Change MAX_SHUTUPS.\n", MAX_SHUTUPS);
    exit(1);
  }

  for (i = 0; i < ShutUpNo; i++)             /* scan table and search for  */
  {                                          /* first free slot            */
    if (shutUpTab[i].ControlByte == FALSE)
    {
      shutUpTab[i].ControlByte = TRUE;       /* fill slot                  */
      shutUpTab[i].func = funkc;
    }
  }
  ShutUpNo++;
}


//°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
/* remove routine, it won't be called before exit */
void subShutUp(void (*funkc)(void))
{
  int i;

  for (i = 0; i < ShutUpNo; i++)                /* search for function */
    if (shutUpTab[i].func == funkc)
      break;

  if (i != ShutUpNo)                            /* valid?              */
  {
    shutUpTab[i].ControlByte = FALSE;           /* do not call!        */
    ShutUpNo--;                                 /* one shutUp less     */
    if (ShutUpNo < 0)
      ShutUpNo = 0;
  }
}

//°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
/* main shutup routine, just call it with error message(s) as a parameter */
void shutUp(char *errmsg, ...)
{
  int           i;
  va_list       argptr;

  for (i = 0; i < ShutUpNo; i++)
    if (shutUpTab[i].ControlByte == TRUE)       /* should we call it? */
      shutUpTab[i].func();

  printf("\n***************** ERROR ***************\n");

  /* show error message */
  va_start(argptr, errmsg);
  vprintf(errmsg, argptr);
  va_end(argptr);

  printf("\n");
  exit (1);                                     /* exit to Disk Op. System */
}                                               /* also known as DOS       */