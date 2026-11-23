/*
 * STUB - placeholder for Derek De Luna's commands.c.
 * Only here so main.c and test.c compile. DELETE THIS FILE when commands.c is ready.
 */
#include <stdio.h>
#include "filesystem.h"

int fsCreate(char type, const char *name) { printf("[stub] fsCreate('%c', \"%s\")\n", type, name); return -1; }
int fsDelete(const char *name)            { printf("[stub] fsDelete(\"%s\")\n", name); return -1; }
int fsOpen(char mode, const char *name)   { printf("[stub] fsOpen('%c', \"%s\")\n", mode, name); return -1; }
int fsClose(void)                         { printf("[stub] fsClose()\n"); return -1; }
int fsRead(int n, char *buf)              { (void)buf; printf("[stub] fsRead(%d)\n", n); return -1; }
int fsWrite(int n, const char *data)      { (void)data; printf("[stub] fsWrite(%d bytes)\n", n); return -1; }
int fsSeek(int pos)                       { printf("[stub] fsSeek(%d)\n", pos); return -1; }
void fsDir(void)                          { printf("[stub] fsDir()\n"); }
int fsCd(const char *name)                { printf("[stub] fsCd(\"%s\")\n", name); return -1; }
void fsRoot(void)                         { printf("[stub] fsRoot()\n"); }
