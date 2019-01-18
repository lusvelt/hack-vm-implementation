#ifndef _HELPER_H_
#define _HELPER_H_

#include "includes.h"

void checkAndLoadArgs(const int argc, char **argv);
int isNaNChar(const char c);
int parseInt(char s[]);
int strsrch(const char *s, const char c);
void saveChanges();

#endif