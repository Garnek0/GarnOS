#ifndef GARN_FNCTL_H
#define GARN_FNCTL_H

#include "../kernel/types.h"

#define O_RDONLY    0
#define O_WRONLY    1
#define O_RDWR      (1 << 1)
#define O_CREAT     (1 << 6)
#define O_EXCL      (1 << 7)
#define O_TRUNC     (1 << 9)
#define O_APPEND    (1 << 10)
#define O_NONBLOCK  (1 << 11)
#define O_DIRECTORY (1 << 16)

#define S_ISUID 0x0800
#define S_ISGID 0x0400
#define S_ISVTX 0x0200
#define S_IRWXU 0x01C0
#define S_IRUSR 0x0100
#define S_IWUSR 0x0080
#define S_IXUSR 0x0040
#define S_IRWXG 0x0038
#define S_IRGRP 0x0020
#define S_IWGRP 0x0010
#define S_IXGRP 0x0008
#define S_IRWXO 0x0007
#define S_IROTH 0x0004
#define S_IWOTH 0x0002
#define S_IXOTH 0x0001

#endif //GARN_FNCTL_H