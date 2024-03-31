#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char** argv){
    int status;

    if(!fork()){
        char* argv[] = {"./bin/shell.elf", NULL};
        char* envp[] = {NULL};

        printf("init: Starting shell...\n\n");

        execve("bin/shell.elf",argv,envp);
        printf("init: failed to start shell\n");
        return -1;
    }

    for(;;){
        waitpid(-1, &status, 0);
    }
}