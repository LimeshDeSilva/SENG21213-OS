#include "fs.h"
#include "vga.h"

static fs_node_t files[MAX_FILES];

static int k_strcmp_fs(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

void fs_init(void) {
    for (int i = 0; i < MAX_FILES; i++) {
        files[i].used = 0;
        files[i].size = 0;
        files[i].name[0] = '\0';
    }

    // Pre-populate with a default readme file
    fs_create("readme.txt");
    const char *intro = "SENG21213-OS RAM Disk Storage initialized successfully!\n";
    uint32_t len = 0;
    while (intro[len]) len++;
    fs_write("readme.txt", intro, len);
}

void fs_list(void) {
    vga_puts("FILENAME             SIZE (BYTES)\n");
    vga_puts("-------------------  ------------\n");
    int count = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used) {
            vga_puts(files[i].name);
            int len = 0;
            while (files[i].name[len]) len++;
            for (int s = len; s < 21; s++) vga_putchar(' ');
            vga_printf("%d B\n", files[i].size);
            count++;
        }
    }
    if (count == 0) {
        vga_puts("  (no files found)\n");
    }
}

int fs_create(const char *name) {
    int free_slot = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used && k_strcmp_fs(files[i].name, name) == 0) {
            return i; // File already exists
        }
        if (!files[i].used && free_slot == -1) {
            free_slot = i;
        }
    }

    if (free_slot == -1) return -1;

    int j = 0;
    while (name[j] && j < MAX_FILENAME - 1) {
        files[free_slot].name[j] = name[j];
        j++;
    }
    files[free_slot].name[j] = '\0';
    files[free_slot].size = 0;
    files[free_slot].used = 1;
    return free_slot;
}

int fs_write(const char *name, const char *data, uint32_t size) {
    int slot = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used && k_strcmp_fs(files[i].name, name) == 0) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        slot = fs_create(name);
        if (slot == -1) return -1;
    }

    if (size > MAX_FILE_SIZE) size = MAX_FILE_SIZE;

    for (uint32_t i = 0; i < size; i++) {
        files[slot].data[i] = data[i];
    }
    files[slot].size = size;
    return size;
}

int fs_read(const char *name, char *buffer, uint32_t buffer_size) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used && k_strcmp_fs(files[i].name, name) == 0) {
            uint32_t to_read = files[i].size;
            if (to_read > buffer_size - 1) to_read = buffer_size - 1;
            for (uint32_t j = 0; j < to_read; j++) {
                buffer[j] = files[i].data[j];
            }
            buffer[to_read] = '\0';
            return to_read;
        }
    }
    return -1; // Not found
}
