#define NUM_SECTORS 100
#define SECTOR_SIZE 512
#define DIR_ENTRIES 25
#define DATA_BYTES 504
#define NAME_LEN 8


union block {
    struct {
        int back;
        int frwd;
        int free;
        struct {
            char type;
            char name[9];
            int link;
            short size;
        } dir[25];
    } directory;
    struct {
        int backf;
        int frwdf;
        char userdata[504];
    } data;
};

struct nodeType {
    int info;
    struct nodeType *link;
};

struct openFile {
    int isOpen;
    int dirSector;
    int entry;
    char mode;
    int pos;
};

//global variables from other files
extern union block sector[NUM_SECTORS];
extern struct nodeType *freeList;
extern int curDir;
extern struct openFile openF;

int checkSizes(void);
void initDisk(void);
void initDir(int s, int back);
void buildList(void);
int Remove(void);
void Insert(int s);
void freeAllNodes(void);

int fsCreate(char type, const char *name);
int fsDelete(const char *name);
int fsOpen(char mode, const char *name);
int fsClose(void);
int fsRead(int n, char *buf);
int fsWrite(int n, const char *data);
int fsSeek(int pos);
void fsDir(void);
int fsCd(const char *name);
void fsRoot(void);

