static FileEntry file_table[MAX_FILES];

void fs_init(void)
{
    memset(file_table, 0, sizeof file_table);
    disk_init();
}

static FileEntry *find_file(const char *name)
{
    for(int i = 0; i < MAX_FILES; i++)
        if(file_table[i].in_use && strcmp(file_table[i].name, name) == 0)
            return &file_table[i];
        return NULL;
}

// free every block in a files chain and reset to empty
static void release_chain(FileEntry *f)
{
    int b = f ->first_block, next;
    char tmp[BLOCK_SIZE];
    while(b != END_OF_FILE)
    {
        if(disk_read_block(b, tmp, &next) != 0) break;
        free_block(b);
        b = next;
    }
    f ->first_block = END_OF_FILE;
    f ->size = 0;
}

// CREATE: adds a new empty file to file table
int fs_create(const char *name)
{
    if(!name || name[0] == '\0' || strlen(name) >= MAX_NAME_LEN)
        return ERR_BAD_NAME;
        
    if( find_file(name)) return ERR_EXISTS;
    
    for(int i = 0; i < MAX_FILES; i++)
    {
        if(!file_table[i].in_use)
        {
            strcpy(file_table[i].name, name);
            file_table[i].size = 0;
            file_table[i].first_block = END_OF_FILE;
            file_table[i].in_use = 1;
            return 0;
        }
    }
    return ERR_TABLE_FULL;
}

//WRITE: stores data using linked allocation. Overwrites old contents
int fs_write(const char *name, const char *data, int len)
{
    FileEntry *f = find_file(name);
    if(!f) return ERR_NOT_FOUND;
    if(len < 0) return ERR_BAD_NAME;
    
    release_chain(f);

    if(len == 0) return 0;
    
    int blocks_needed = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;
    
    if(blocks_needed > free_block_count()) return ERR_NO_SPACE;
    
    int chain[NUM_BLOCKS];
    for(int i = 0; i < blocks_needed; i++)
    {
        chain[i] = alloc_block();
        if(chain[i] < 0)
        {
            for(int j = 0; j < i; j++)
            free_block(chain[j]);
            return ERR_NO_SPACE;
        }
    }
    
    for(int i = 0; i < blocks_needed; i++)
    {
        int offset = i * BLOCK_SIZE;
        int chunk = (len - offset < BLOCK_SIZE) ? len - offset : BLOCK_SIZE;
        int next = (i + 1 < blocks_needed) ? chain[i + 1] : END_OF_FILE;
        
        disk_write_block(chain[i], data + offset, chunk, next);
    }
    f ->first_block = chain[0];
    f ->size = len;
    return 0;
}

//READ: follows the block chain and copies the files bytes into buf.
int fs_read(const char *name, char *buf, int buf_size)
{
    FileEntry *f = find_file(name);
    if(!f) return ERR_NOT_FOUND;
    
    int to_read = (f->size < buf_size) ? f->size : buf_size;
    int copied = 0, b = f->first_block, next;
    char block[BLOCK_SIZE];
    
    while(b != END_OF_FILE && copied < to_read)
    {
        if(disk_read_block(b, block, &next) != 0) return -6;
        int chunk = (to_read - copied < BLOCK_SIZE) ? to_read - copied : BLOCK_SIZE;
        memcpy(buf + copied, block, chunk);
        copied += chunk;
        b = next;
    }
    return copied;
}

//DELETE: remove a file and release its blocks. not completed yet
int fs_delete(const char *name)
{ 
    (void)name; return -99; 
}
void fs_list(void)
{
    printf("(list not implemented yet)\n");
}

const char *fs_strerror(int c)
{
    switch (c)
    {
       case 0: return "OK";
       case ERR_EXISTS: return "file already exists";
       case ERR_TABLE_FULL: return "file table full";
       case ERR_NOT_FOUND: return "file not found";
       case ERR_NO_SPACE: return "not enough free disk blocks";
       case ERR_BAD_NAME: return "invalid name/argument";
       default: return "error";
    }
}
