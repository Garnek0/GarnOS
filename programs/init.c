#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define SHELL_PATH "/bin/shell.elf"

int main(int argc, char** argv){
    printf("Welcome to GarnOS!\n");

    int status;

    if(!fork()){
        char* argv[] = {SHELL_PATH, NULL};
        char* envp[] = {NULL};

        printf("init: Starting shell...\n\n");

        execve(SHELL_PATH,argv,envp);
    	printf("init: "SHELL_PATH": ");
        perror(NULL);
        return -1;
	}

    for(;;){
        waitpid(-1, &status, 0);
    }
}
