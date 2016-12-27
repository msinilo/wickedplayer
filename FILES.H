#ifndef __FILES_H
#define __FILES_H

#include "shutup.h"

int SafeOpenW(char *fname);
int SafeOpenR(char *fname);
FILE *SafeFOpen(char *fname, const char *tryb);
void SafeRead(int handle, void *buf, udword ile);
void SafeWrite(int handle, void *buf, udword ile);
void SafeFRead(void *buf, udword ile, FILE *f);
void SafeFWrite(void *buf, udword ile, FILE *f);
udword fsize(FILE *f);
udword SafeSize(int handle);
void *SafeLoad(char *fname);
udword SafeLoadS(char *fname, void **bufptr);
void *SafeFLoad(char *fname);
udword SafeFLoadS(char *fname, void **bufptr);
void SafeSave(char *fname, void *buf, udword ile);
void SafeFSave(char *fname, void *buf, udword ile);

#endif