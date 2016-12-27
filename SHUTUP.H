#ifndef __SHUTUP_H
#define __SHUTUP_H

void vmode3(void);
void addShutUp (void (*funkc)(void));
void subShutUp (void (*funkc)(void));
void shutUp(char *errmsg, ...);

#endif