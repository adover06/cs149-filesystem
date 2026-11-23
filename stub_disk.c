#include <stdio.h>
#include "filesystem.h"

union block sector[NUM_SECTORS];
struct nodeType *freeList = NULL;
int curDir = 0;
struct openFile openF;

int  checkSizes(void)          { return 1; }
void initDisk(void)            { curDir = 0; openF.isOpen = 0; }
void initDir(int s, int back)  { (void)s; (void)back; }
void buildList(void)           { }
int  Remove(void)              { return -1; }
void Insert(int s)             { (void)s; }
void freeAllNodes(void)        { }
