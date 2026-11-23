#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filesystem.h"

union block sector[NUM_SECTORS];
struct nodeType *freeList = NULL;
int curDir = 0;
struct openFile openF;

int checkSizes(void)
{
    int entrySize = sizeof(sector[0].directory.dir[0]);
    int blockSize = sizeof(union block);

    if (entrySize != 20 || blockSize != SECTOR_SIZE)
        return 0;
    return 1;
}

void initDir(int s, int back)
{
    int i;

    sector[s].directory.back = back;
    sector[s].directory.frwd = 0;
    sector[s].directory.free = DIR_ENTRIES;

    for (i = 0; i < DIR_ENTRIES; i++) {
        sector[s].directory.dir[i].type = 'F';
        sector[s].directory.dir[i].name[0] = '\0';
        sector[s].directory.dir[i].link = 0;
        sector[s].directory.dir[i].size = 0;
    }
}

void buildList(void)
{
    int s;
    struct nodeType *node;
    struct nodeType *last = NULL;

    freeList = NULL;
    for (s = 1; s < NUM_SECTORS; s++) {
        node = malloc(sizeof(struct nodeType));
        if (node == NULL) {
            printf("Error: out of memory building free list\n");
            exit(1);
        }
        node->info = s;
        node->link = NULL;

        if (last == NULL)
            freeList = node;
        else
            last->link = node;
        last = node;
    }
}

int Remove(void)
{
    struct nodeType *node;
    int s;

    if (freeList == NULL)
        return -1;

    node = freeList;
    s = node->info;
    freeList = node->link;
    free(node);

    memset(&sector[s], 0, sizeof(union block));
    return s;
}

void Insert(int s)
{
    struct nodeType *node;
    struct nodeType *prev = NULL;
    struct nodeType *cur = freeList;

    if (s <= 0 || s >= NUM_SECTORS) {
        printf("Error: Insert(%d) is not a valid sector\n", s);
        return;
    }

    while (cur != NULL && cur->info < s) {
        prev = cur;
        cur = cur->link;
    }
    if (cur != NULL && cur->info == s) {
        printf("Error: sector %d is already free\n", s);
        return;
    }

    node = malloc(sizeof(struct nodeType));
    if (node == NULL) {
        printf("Error: out of memory in Insert\n");
        exit(1);
    }
    node->info = s;
    node->link = cur;

    if (prev == NULL)
        freeList = node;
    else
        prev->link = node;
}

void freeAllNodes(void)
{
    struct nodeType *next;

    while (freeList != NULL) {
        next = freeList->link;
        free(freeList);
        freeList = next;
    }
}

void initDisk(void)
{
    memset(sector, 0, sizeof(sector));

    initDir(0, -1);
    sector[0].directory.frwd = -1;

    freeAllNodes();
    buildList();

    curDir = 0;

    openF.isOpen = 0;
    openF.dirSector = 0;
    openF.entry = 0;
    openF.mode = ' ';
    openF.pos = 0;
}
