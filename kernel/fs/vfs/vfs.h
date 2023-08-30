#ifndef VFS_H
#define VFS_H

#include <types.h>

#define VFS_MAX_FILESYSTEMS 256

#define VFS_FS_AVAIL 1

#define VFS_FILE_ACCESS_R 0
#define VFS_FILE_ACCESS_W 1
#define VFS_FILE_ACCESS_RW 2


typedef struct {
    uint64_t* address;
    size_t size;
    uint8_t access; //0 - read, 1 - write, 2 - read/write;
    uint8_t fsNumber;
    size_t seek;
} vfs_file_t;

typedef struct {
    void* context;
    uint8_t flags; //bit 0 - available
    uint8_t mountNumber; //256 filesystems is probably be enough...
    char* name;

    vfs_file_t* (*open)(char* path, uint8_t access, uint8_t fsNumber);
    void (*close)(vfs_file_t* file);
    void (*read)(vfs_file_t* file, size_t size, void* buf);
    void (*write)(vfs_file_t* file, size_t size, void* buf);
} vfs_fs_t;

void vfs_add(vfs_fs_t fs);
void vfs_remove(uint8_t index);
vfs_file_t* vfs_open(char* path, uint8_t access);
void vfs_close(vfs_file_t* file);
void vfs_read(vfs_file_t* file, size_t size, void* buf);
void vfs_write(vfs_file_t* file, size_t size, void* buf);
void vfs_seek(vfs_file_t* file, size_t seekPos);

#endif //VFS_H