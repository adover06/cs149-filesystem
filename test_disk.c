#include <stdio.h>
#include "filesystem.h"

static int passes = 0, fails = 0;

#define CHECK(cond, msg) do { \
    if (cond) { passes++; printf("  PASS  %s\n", msg); } \
    else      { fails++;  printf("  FAIL  %s\n", msg); } \
} while (0)

static int countFree(void)
{
    int seen[NUM_SECTORS] = {0}, n = 0;
    struct nodeType *p;
    for (p = freeList; p != NULL; p = p->link) {
        if (p->info <= 0 || p->info >= NUM_SECTORS || seen[p->info])
            return -1;
        seen[p->info] = 1;
        n++;
    }
    return n;
}

static int isFree(int s)
{
    struct nodeType *p;
    for (p = freeList; p != NULL; p = p->link)
        if (p->info == s)
            return 1;
    return 0;
}

static void test_sizes(void)
{
    printf("sizes\n");
    CHECK(sizeof(sector[0].directory.dir[0]) == 20, "directory entry is 20 bytes");
    CHECK(sizeof(union block) == 512, "union block is 512 bytes");
    CHECK(checkSizes() == 1, "checkSizes returns 1");
}

static void test_init(void)
{
    int i, allF = 1;
    printf("initDisk\n");
    initDisk();
    for (i = 0; i < DIR_ENTRIES; i++)
        if (sector[0].directory.dir[i].type != 'F' || sector[0].directory.dir[i].link != 0)
            allF = 0;
    CHECK(sector[0].directory.back == -1 && sector[0].directory.frwd == -1, "root back/frwd = -1");
    CHECK(sector[0].directory.free == 25, "root free = 25");
    CHECK(allF, "all 25 root entries are F with link 0");
    CHECK(countFree() == 99, "free list has 99 sectors");
    CHECK(!isFree(0), "sector 0 is not on the free list");
    CHECK(freeList != NULL && freeList->info == 1, "free list starts at sector 1");
    CHECK(curDir == 0, "current directory is root");
    CHECK(openF.isOpen == 0, "no file open");
}

static void test_remove_insert(void)
{
    int a, b, c;
    printf("Remove / Insert\n");
    initDisk();
    a = Remove();
    b = Remove();
    c = Remove();
    CHECK(a == 1 && b == 2 && c == 3, "Remove hands out 1, 2, 3");
    CHECK(countFree() == 96 && !isFree(a) && !isFree(b), "removed sectors are off the list");

    Insert(b);
    CHECK(countFree() == 97 && isFree(b), "Insert puts sector 2 back");
    Insert(b);
    CHECK(countFree() == 97, "inserting 2 again does not duplicate it");
    Insert(0);
    CHECK(countFree() == 97 && !isFree(0), "Insert(0) is ignored");
    Insert(100);
    Insert(-5);
    CHECK(countFree() == 97, "out of range sectors are ignored");
    CHECK(Remove() == 2, "lowest free sector (2) is used next");

    Insert(a);
    Insert(c);
    CHECK(countFree() == 98 && freeList->info == 1, "list stays in order after inserts");
}

static void test_remove_clears(void)
{
    int s;
    printf("Remove clears the sector\n");
    initDisk();
    sector[1].data.frwdf = 77;
    sector[1].data.userdata[0] = 'x';
    s = Remove();
    CHECK(s == 1 && sector[1].data.frwdf == 0 && sector[1].data.userdata[0] == 0,
          "sector handed out by Remove is all zeros");
}

static void test_full(void)
{
    int i, ok = 1;
    printf("run out of sectors\n");
    initDisk();
    for (i = 1; i < NUM_SECTORS; i++)
        if (Remove() != i)
            ok = 0;
    CHECK(ok, "99 Removes return 1..99");
    CHECK(freeList == NULL && countFree() == 0, "free list is empty");
    CHECK(Remove() == -1, "Remove returns -1 when the disk is full");
    Insert(50);
    CHECK(countFree() == 1 && Remove() == 50, "a freed sector can be used again");
}

static void test_initdir(void)
{
    int s, i, allF = 1;
    printf("initDir\n");
    initDisk();
    s = Remove();
    initDir(s, 0);
    for (i = 0; i < DIR_ENTRIES; i++)
        if (sector[s].directory.dir[i].type != 'F' || sector[s].directory.dir[i].name[0] != '\0')
            allF = 0;
    CHECK(sector[s].directory.back == 0 && sector[s].directory.frwd == 0,
          "new directory sector: back = 0 (given), frwd = 0");
    CHECK(sector[s].directory.free == 25 && allF, "new directory sector: 25 free F entries");
}

static void test_reinit(void)
{
    printf("initDisk twice\n");
    initDisk();
    Remove();
    Remove();
    sector[0].directory.dir[0].type = 'U';
    initDisk();
    CHECK(countFree() == 99, "second initDisk rebuilds a full free list");
    CHECK(sector[0].directory.dir[0].type == 'F', "second initDisk clears the root");
}

int main(void)
{
    test_sizes();
    test_init();
    test_remove_insert();
    test_remove_clears();
    test_full();
    test_initdir();
    test_reinit();
    freeAllNodes();
    CHECK(freeList == NULL, "freeAllNodes empties the list");
    printf("\n%d passed, %d failed\n", passes, fails);
    return fails != 0;
}
