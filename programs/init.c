#include <unistd.h>
#include <string.h>

void _start(){
    char* bufShellFailed = "init: failed to start shell\n";
    int status;

    if(fork() == 0){
        char* argv[] = {"./bin/shell.elf", NULL};
        char* envp[] = {NULL};
        execve("bin/shell.elf",argv,envp);
        write(1, bufShellFailed, strlen(bufShellFailed));
        exit(0);
    }

    for(;;){
        waitpid(-1, &status, 0);
    }
}