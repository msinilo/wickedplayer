// File Routines v1.2, Programmed by Yarpen of Swirl
// Copyright (C) 1997, Maciej Sini’o
// Some tricks from id Software and their QuakeC compiler, thanx!
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <errno.h>
#include <types.h>
#include "shutup.h"

static void *F_SafeMalloc(long size, const char *file, unsigned line);
#define SMALLOC(s) F_SafeMalloc(s, __FILE__, __LINE__)

//°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
static void *F_SafeMalloc(long size, const char *file, unsigned line)
{
  void          *tmp;

  tmp = malloc(size);
  if (!tmp)
    shutUp("Cannot allocate memory! I need %lu bytes! [%s - %d]\n", size,
      file, line);

  return tmp;
}

//°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
int SafeOpenW(char *fname)
{
  int           handle;

  umask(0);

  handle = open(fname, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0666);

  if (handle == -1)
    shutUp("File %s opening error: %s!", strupr(fname), strerror(errno));

  return handle;
}


//°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
int SafeOpenR(char *fname)
{
  int           handle;

  handle = open(fname, O_RDONLY | O_BINARY);

  if (handle == -1)
    shutUp("File %s opening error: %s!", strupr(fname), strerror(errno));

  return handle;
}

//°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
FILE *SafeFOpen(char *fname, const char *tryb)
{
  FILE          *temp;

  temp = fopen(fname, tryb);

  if (!temp)
    shutUp("File %s opening error: %s!", strupr(fname), strerror(errno));

  return temp;
}


//°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
void SafeRead(int handle, void *buf, udword ile)
{
  if (read(handle, buf, ile) != ile)
    shutUp("File reading error: %s!", strerror(errno));
}

//°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
void SafeWrite(int handle, void *buf, udword ile)
{
  if (write(handle, buf, ile) != ile)
    shutUp("File writing error: %s!", strerror(errno));
}

//°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
void SafeFRead(void *buf, udword ile, FILE *f)
{
  if (fread(buf, ile, 1, f) != 1)
    shutUp("File reading error at %d: %s!", ftell(f), strerror(errno));
}

//°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
void SafeFWrite(void *buf, udword ile, FILE *f)
{
  if (fwrite(buf, ile, 1, f) != 1)
    shutUp("File writing error at %d: %s!", ftell(f), strerror(errno));
}


//°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
udword fsize(FILE *f)
{
  udword        temp;
  udword        len;

  temp = ftell(f);
  fseek(f, 0, SEEK_END);
  len = ftell(f);
  fseek(f, temp, SEEK_SET);

  return len;
}


//°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
udword SafeSize(int handle)
{
  struct stat   fileinfo;

  if (fstat(handle, &fileinfo) == -1)
    shutUp("Error fstating!");

  return fileinfo.st_size;
}


//°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
void *SafeLoad(char *fname)
{
  int           handle;
  udword        len;
  void          *buf;

  handle = SafeOpenR(fname);
  len = SafeSize(handle);
  buf = (void *)SMALLOC(len);
  SafeRead(handle, buf, len);
  close(handle);

  return buf;
}

//°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
udword SafeLoadS(char *fname, void **bufptr)
{
  int           handle;
  udword        len;
  void          *buf;

  handle = SafeOpenR(fname);
  len = SafeSize(handle);
  buf = (void *)SMALLOC(len + 1);
  ((char *)buf)[len] = 0;
  SafeRead(handle, buf, len);
  close(handle);

  *bufptr = buf;
  return len;
}


//°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
void *SafeFLoad(char *fname)
{
  FILE          *f;
  udword        len;
  void          *buf;

  f = SafeFOpen(fname, "rb");
  len = fsize(f);
  buf = (void *)SMALLOC(len);
  SafeFRead(buf, len, f);
  fclose(f);

  return buf;
}

//°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
udword SafeFLoadS(char *fname, void **bufptr)
{
  FILE          *f;
  udword        len;
  void          *buf;

  f = SafeFOpen(fname, "rb");
  len = fsize(f);
  buf = (void *)SMALLOC(len + 1);
  ((char *)buf)[len] = 0;
  SafeFRead(buf, len, f);
  fclose(f);

  *bufptr = buf;
  return len;
}


//°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
void SafeSave(char *fname, void *buf, udword ile)
{
  int           handle;

  handle = SafeOpenW(fname);
  SafeWrite(handle, buf, ile);
  close(handle);
}


//°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
void SafeFSave(char *fname, void *buf, udword ile)
{
  FILE          *f;

  f = SafeFOpen(fname, "wb");
  SafeFWrite(buf, ile, f);
  fclose(f);
}