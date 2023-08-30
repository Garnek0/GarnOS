#ifndef INITRD_H
#define INITRD_H

#include <types.h>

#define INITRD_FILENAME "initrd.grd"

typedef struct {
    char filename[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mTime[12];
    char checksum[8];
    char typeFlag[1];
}__attribute__((packed)) initrd_tar_header_t;

void initrd_init();
void initrd_remove();

#endif //INITRD_H