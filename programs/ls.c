#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* get_ext(char* filename){
    char* ext = strrchr(filename, '.');
    if(!ext || !strcmp(ext, filename)) return "";
    return &ext[1];
}

int ls(const char* path){
    struct dirent* dirent;

    DIR* dir = opendir(path);
    if(!dir){
        printf("ls: cant access \'%s\': ", path);
        perror(NULL);
        return -1;
    }

    dirent = readdir(dir); if(!dirent) return 0; //Directory is empty (or something terribly wrong happened)

    do {
        if(!strcmp(dirent->d_name, "..") || !strcmp(dirent->d_name, ".")){
            dirent = readdir(dir);
            continue;
        }

        if(dirent->d_type == DT_REG){
            if(!strcmp(get_ext(dirent->d_name), "elf") || !strcmp(get_ext(dirent->d_name), "sys") || !strcmp(get_ext(dirent->d_name), "bin")){
                printf("\e[38;2;0;255;0m%s\e[38;2;255;255;255m\n", dirent->d_name);
            } else {
                printf("%s\n", dirent->d_name);
            }
        } else if(dirent->d_type == DT_DIR){
            printf("\e[38;2;0;0;255m%s\e[38;2;255;255;255m\n", dirent->d_name);
		} else if(dirent->d_type == DT_CHR || dirent->d_type == DT_BLK){
			printf("\e[38;2;100;100;255m%s\e[38;2;255;255;255m\n", dirent->d_name);
		} else {
			printf("%s (UNKNOWN TYPE)\n", dirent->d_name);
		}
        dirent = readdir(dir);
    } while(dirent);

    closedir(dir);

	return 0;
}

int main(int argc, char** argv){
	int ret = 0;
    int retls = 0;

    if(argc == 1){
        retls = ls(".");
        if(retls != 0) ret = retls;
    } else if(argc == 2){
        retls = ls(argv[1]);
        if(retls != 0) ret = retls;
    } else if(argc >= 3){
        DIR* check;

        for(int i = 1; i < argc; i++){
            check = opendir(argv[i]);
            if(!check){
                printf("ls: cant access \'%s\': ", argv[i]);
                perror(NULL);
                ret = -1;
                continue;
            }
            closedir(check);

            printf("%s:\n", argv[i]);
            retls = ls(argv[i]);
            if(retls != 0) ret = retls;
            printf("\n");
        }
    }

    return ret;
}
