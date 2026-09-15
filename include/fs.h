#ifndef FS_H
#define FS_H

#include "types.h"

#define MAX_FILES       16
#define MAX_FILENAME    32
#define MAX_FILE_SIZE   1024

typedef struct {
    char     name[MAX_FILENAME];
    uint32_t size;
    uint8_t  used;
    char     data[MAX_FILE_SIZE];
} fs_node_t;

void fs_init(void);
void fs_list(void);
int  fs_create(const char *name);
int  fs_write(const char *name, const char *data, uint32_t size);
int  fs_read(const char *name, char *buffer, uint32_t buffer_size);

#endif
