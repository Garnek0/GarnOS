#include <unistd.h>
#include <sys/mman.h>
#include <dirent.h>
#include <fnctl.h>
#include <string.h>

void _start(){
    struct garn_dirent64* dirent;

    size_t bufSize = 0x1000;
    char* buf = mmap(NULL, bufSize, PROT_WRITE | PROT_READ, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
    memset(buf, 0, bufSize);

    int fd = open(".", O_RDONLY | O_DIRECTORY, 0);
    if(fd < 0){
        write(2, "ls: ", 4);
        switch(fd){
            case -2: //ENOENT
                write(2, "No such file or directory.\n", 27);
                break;
            case -20: //ENOTDIR
                write(2, "Not a directory.\n", 17);
                break;
            default:
                write(2, "Unknown error.\n", 15);
                break;
        }
    }

    getdents64(fd, buf, bufSize);

    int i = 0;

    do {
        dirent = (struct garn_dirent64_t*)(buf+i);
        write(1, dirent->name, strlen(dirent->name));
        write(1, "\n", 1);
        i+=dirent->recordLength;
    } while(dirent->recordLength);

    
    exit(0);
}