#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

void ls(const char* path){
    struct dirent* dirent;

    size_t bufSize = 0x1000;
    char* buf = malloc(bufSize);
    memset(buf, 0, bufSize);

    //TODO: fix this

    DIR* dir = opendir(path);
    if(!dir){
        printf("ls: cant access \'%s\': ", path);
        perror(NULL);
        return -1;
    }

    dirent = readdir(dir); if(!dir) return 0; //Directory is empty

    do {
        printf("%s\n", dirent->d_name, dirent->d_reclen);
        dirent = readdir(dir);
    } while(dirent);

    closedir(dir);
}

int main(int argc, char** argv){
    if(argc == 1){
        ls(".");
    } else if(argc == 2){
        ls(argv[1]);
    } else if(argc >= 3){
        DIR* check;

        for(int i = 1; i < argc; i++){
            check = opendir(argv[i]);
            if(!check){
                printf("ls: cant access \'%s\': ", argv[i]);
                perror(NULL);
                continue;
            }
            closedir(check);

            printf("%s:\n", argv[i]);
            ls(argv[i]);
            printf("\n");
        }
    }

    
    
    return 0;
}