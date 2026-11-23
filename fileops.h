#ifndef FILESOPS_H
#define FILESOPS_H

#define MAX_FILES 20
#define MAX_NAME_LEN 32

typedef struct
{
    char name[MAX_NAME_LEN];
    int size; //bytes stored
    int first_block; // head of linked block chain
    int in_use;
    
} FileEntry;

#define ERR_EXISTS (-1)
#define ERR_TABLE_FULL (-2)
#define ERR_NOT_FOUND (-3)
#define ERR_NO_SPACE (-4)
#define ERR_BAD_NAME (-5)

void fs_init(void);
int fs_create(const char *name);
int fs_write(const char *name, const char *data, int len);
int fs_read(const char *name, char *buf, int buf_size);

int fs_delete(const char *name);
void fs_list(void);
const char *fs_strerror(int code);

#endif