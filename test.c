#include <stdio.h>
#include <string.h>
#include "filesystem.h"

int passed = 0, failed = 0;
char buf[2000];

void check(int ok, char *name)
{
    if (ok) {
        printf("PASS: %s\n", name);
        passed++;
    } else {
        printf("FAIL: %s\n", name);
        failed++;
    }
}

int countFree(void)
{
    int n = 0;
    struct nodeType *p = freeList;
    while (p != NULL) {
        n++;
        p = p->link;
    }
    return n;
}

int main(void)
{
    int i;

    initDisk();
    fsCreate('U', "A");
    fsOpen('U', "A");
    fsWrite(5, "HELLO");
    fsSeek(0);
    fsRead(5, buf);
    buf[5] = '\0';
    fsClose();
    check(strcmp(buf, "HELLO") == 0, "write HELLO and read it back");

    initDisk();
    fsCreate('U', "A");
    check(fsCreate('U', "A") == -1, "creating A twice fails");

    initDisk();
    check(fsOpen('I', "NOPE") == -1, "opening a missing file fails");

    initDisk();
    for (i = 0; i < 1200; i++)
        buf[i] = 'x';
    fsCreate('U', "A");
    fsOpen('O', "A");
    fsWrite(1200, buf);
    fsClose();
    check(countFree() == 96, "1200 bytes uses 3 sectors");
    fsDelete("A");
    check(countFree() == 99, "delete puts all 3 sectors back on the free list");

    freeAllNodes();
    printf("\n%d passed, %d failed\n", passed, failed);
    return 0;
}
