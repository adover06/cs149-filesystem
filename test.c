/*
 * test.c - test cases (Andrew Dover)
 * Build: gcc -Wall -o test test.c disk.c commands.c
 * All test files live in the root directory (sector 0).
 */
#include <stdio.h>
#include <string.h>
#include "filesystem.h"

static int passes = 0, fails = 0;

#define CHECK(cond, msg) do { \
    if (cond) { passes++; printf("  PASS  %s\n", msg); } \
    else      { fails++;  printf("  FAIL  %s\n", msg); } \
} while (0)

static char buf[40000], expect[40000];

/* Start every test from an empty disk. */
static void reset(void)
{
    freeAllNodes();
    initDisk();
}

/* Number of sectors in the free list, or -1 if it contains sector 0 or a duplicate. */
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

/* Index of name in the root directory, or -1. */
static int findEntry(const char *name)
{
    int i;
    for (i = 0; i < DIR_ENTRIES; i++)
        if (sector[0].directory.dir[i].type != 'F' &&
            strcmp(sector[0].directory.dir[i].name, name) == 0)
            return i;
    return -1;
}

static int sizeOf(const char *name)
{
    int i = findEntry(name);
    return i < 0 ? -1 : sector[0].directory.dir[i].size;
}

/* Fill b with n bytes of a recognizable pattern. */
static void pattern(char *b, int n, char start)
{
    int i;
    for (i = 0; i < n; i++)
        b[i] = start + i % 26;
}

/* Create name and write n bytes of pattern to it. */
static int makeFile(const char *name, int n, char start)
{
    int w;
    fsCreate('U', name);
    pattern(buf, n, start);
    fsOpen('O', name);
    w = fsWrite(n, buf);
    fsClose();
    return w;
}

static void test_init(void)
{
    int i, allF = 1;
    printf("init\n");
    CHECK(checkSizes() == 1, "checkSizes passes (dir entry 20, union 512)");
    reset();
    for (i = 0; i < DIR_ENTRIES; i++)
        if (sector[0].directory.dir[i].type != 'F')
            allF = 0;
    CHECK(countFree() == 99, "free list holds sectors 1..99");
    CHECK(sector[0].directory.free == 25 && allF, "root has 25 free entries, all F");
}

static void test_basic(void)
{
    printf("create / write / read\n");
    reset();
    CHECK(fsCreate('U', "A") == 0, "CREATE U A");
    CHECK(findEntry("A") >= 0 && sizeOf("A") == 0 && sector[0].directory.dir[findEntry("A")].link == 0,
          "new file has size 0 and link 0");
    CHECK(sector[0].directory.free == 24, "root free count drops to 24");
    CHECK(fsOpen('U', "A") == 0, "OPEN U A");
    CHECK(fsWrite(5, "HELLO") == 5, "WRITE 5 HELLO");
    CHECK(fsWrite(6, " WORLD") == 6, "WRITE 6 ' WORLD' keeps the space");
    CHECK(fsSeek(0) == 0, "SEEK 0");
    memset(buf, 0, 20);
    CHECK(fsRead(100, buf) == 11 && memcmp(buf, "HELLO WORLD", 11) == 0,
          "READ 100 returns the 11 bytes written");
    CHECK(sizeOf("A") == 11, "size is 11");
    CHECK(countFree() == 98, "one sector used");
    CHECK(fsClose() == 0, "CLOSE");
}

static void test_read_missing(void)
{
    printf("read a file that doesn't exist\n");
    reset();
    CHECK(fsOpen('I', "NOPE") == -1, "OPEN I NOPE fails");
    CHECK(fsRead(10, buf) == -1, "READ with no open file fails");
}

static void test_duplicate(void)
{
    printf("duplicate name\n");
    reset();
    fsCreate('U', "A");
    CHECK(fsCreate('U', "A") == -1, "second CREATE U A rejected");
    CHECK(fsCreate('D', "A") == -1, "CREATE D A rejected too");
    CHECK(sector[0].directory.free == 24, "only one entry used");
}

static void test_delete_frees(void)
{
    printf("delete returns sectors to the free list\n");
    reset();
    CHECK(makeFile("A", 1200, 'a') == 1200, "write 1200 bytes");
    CHECK(countFree() == 96, "1200 bytes uses 3 sectors");
    CHECK(fsDelete("A") == 0, "DELETE A");
    CHECK(countFree() == 99, "all 3 sectors back in free list");
    CHECK(findEntry("A") == -1 && sector[0].directory.free == 25, "entry marked F");
}

static void test_too_big_and_full(void)
{
    printf("write larger than free space / fill the disk\n");
    reset();
    /* size is a short, so one file can't hold 99 sectors; use two files */
    CHECK(makeFile("A", 50 * DATA_BYTES, 'a') == 50 * DATA_BYTES, "A takes 50 sectors");
    CHECK(countFree() == 49, "49 sectors left");
    CHECK(makeFile("B", 50 * DATA_BYTES, 'a') == 49 * DATA_BYTES,
          "writing 50 sectors to B only writes the 49 that are free");
    CHECK(sizeOf("B") == 49 * DATA_BYTES, "B size matches bytes actually written");
    CHECK(countFree() == 0, "disk is full");
    CHECK(fsCreate('D', "X") == -1, "CREATE D fails when no sector is free");
    CHECK(fsCreate('U', "C") == 0, "CREATE U still works (needs no sector)");
    fsOpen('O', "C");
    CHECK(fsWrite(1, "z") < 1, "write to full disk writes nothing");
    fsClose();
    CHECK(fsDelete("A") == 0 && countFree() == 50, "deleting A frees its 50 sectors");
}

static void test_fragmentation(void)
{
    int s1, s2;
    printf("fragmentation (non-contiguous file)\n");
    reset();
    makeFile("A", DATA_BYTES, 'a');      /* A gets a sector */
    makeFile("B", DATA_BYTES, 'n');      /* B gets the next one */
    pattern(buf, DATA_BYTES, 'A');       /* append a second sector to A */
    fsOpen('O', "A");
    CHECK(fsWrite(DATA_BYTES, buf) == DATA_BYTES, "append 504 more bytes to A");
    fsClose();

    s1 = findEntry("A") < 0 ? 0 : sector[0].directory.dir[findEntry("A")].link;
    s2 = (s1 > 0 && s1 < NUM_SECTORS) ? sector[s1].data.frwdf : 0;
    CHECK(s2 > 0 && s2 < NUM_SECTORS && s2 != s1 + 1, "A's two sectors are not next to each other");
    CHECK(s2 > 0 && s2 < NUM_SECTORS && sector[s2].data.backf == s1 && sector[s2].data.frwdf == 0,
          "back/forward links correct");

    pattern(expect, DATA_BYTES, 'a');
    pattern(expect + DATA_BYTES, DATA_BYTES, 'A');
    fsOpen('I', "A");
    CHECK(fsRead(2 * DATA_BYTES, buf) == 2 * DATA_BYTES &&
          memcmp(buf, expect, 2 * DATA_BYTES) == 0, "READ follows the link across sectors");
    CHECK(fsSeek(600) == 0 && fsRead(1, buf) == 1 && buf[0] == expect[600],
          "SEEK 600 lands at offset 96 of the second sector");
    fsClose();

    CHECK(fsDelete("B") == 0 && countFree() == 97, "deleting B (in the middle) frees 1 sector");
    fsOpen('I', "A");
    CHECK(fsRead(2 * DATA_BYTES, buf) == 2 * DATA_BYTES &&
          memcmp(buf, expect, 2 * DATA_BYTES) == 0, "A unchanged after B deleted");
    fsClose();
}

int main(void)
{
    test_init();
    test_basic();
    test_read_missing();
    test_duplicate();
    test_delete_frees();
    test_too_big_and_full();
    test_fragmentation();
    freeAllNodes();
    printf("\n%d passed, %d failed\n", passes, fails);
    return fails != 0;
}
