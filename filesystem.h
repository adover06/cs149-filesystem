/*
 * filesystem.h - shared contract for the CS-149 file system project.
 *
 * Layout follows the Week 14 "Project Description" on Canvas.
 *
 *   disk.c      (Andrew Porasl) - disk, free-sector list, directory init
 *   commands.c  (Derek De Luna) - CREATE/DELETE/OPEN/CLOSE/READ/WRITE/SEEK/DIR/CD/ROOT
 *   main.c      (Andrew Dover)  - command loop that calls the functions below
 *   test.c      (Andrew Dover)  - test cases
 *
 * disk.c is done. Until commands.c exists, stub_commands.c stands in.
 */
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#define NUM_SECTORS 100   /* sectors 0..99, sector 0 = root directory */
#define SECTOR_SIZE 512
#define DIR_ENTRIES 25    /* entries per directory sector */
#define DATA_BYTES  504   /* user data per file sector */
#define NAME_LEN    8     /* max name length (name[9] holds the '\0') */

/* One disk sector: either a directory sector or a file-data sector. */
union block {
    struct {
        int back;              /* previous sector of this directory */
        int frwd;              /* next sector of this directory, 0 = end */
        int free;              /* unused entries in THIS sector */
        struct {
            char type;         /* 'U' file, 'D' directory, 'F' unused */
            char name[9];
            int link;          /* first sector (0 = empty file) */
            short size;        /* file size in bytes, 0 for directories */
        } dir[25];
    } directory;
    struct {
        int backf;             /* previous sector of this file, 0 = none */
        int frwdf;             /* next sector of this file, 0 = none */
        char userdata[504];
    } data;
};

/* Free-sector list node: one free sector number per node. */
struct nodeType {
    int info;
    struct nodeType *link;
};

/* The one open file. */
struct openFile {
    int isOpen;      /* 0 = no file open */
    int dirSector;   /* directory sector holding the file's entry */
    int entry;       /* index of the entry in that sector (0..24) */
    char mode;       /* 'I' read, 'O' write (append), 'U' read+write */
    int pos;         /* current byte position (separate from size) */
};

/* Globals (defined in disk.c) */
extern union block sector[NUM_SECTORS];
extern struct nodeType *freeList;   /* head of free-sector list */
extern int curDir;                  /* first sector of current directory */
extern struct openFile openF;

/* ---- disk.c (Porasl) ---- */
int  checkSizes(void);   /* 1 if dir entry == 20 and union == 512 bytes, else 0 */
void initDisk(void);     /* root in sector[0], buildList, curDir = 0, no file open */
void initDir(int s, int back);  /* set up sector s as an empty directory sector */
void buildList(void);    /* free list = sectors 1..99 */
int  Remove(void);       /* take a free sector off the list, -1 if none left */
void Insert(int s);      /* put sector s back on the free list */
void freeAllNodes(void); /* free every node in the list (called on EXIT) */

/* ---- commands.c (Derek) ----
 * Return 0 on success and -1 on error unless noted. */
int  fsCreate(char type, const char *name);  /* type 'U' or 'D' */
int  fsDelete(const char *name);             /* file, or empty directory */
int  fsOpen(char mode, const char *name);    /* mode 'I', 'O', 'U' */
int  fsClose(void);
int  fsRead(int n, char *buf);          /* returns bytes read, -1 on error */
int  fsWrite(int n, const char *data);  /* returns bytes written (less than n
                                           if the disk fills up), -1 on error */
int  fsSeek(int pos);                   /* 0 <= pos <= size */
void fsDir(void);
int  fsCd(const char *name);
void fsRoot(void);

#endif
