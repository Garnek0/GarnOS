#include <unistd.h>
#include <string.h>

void _start(){
    char* bufWelcome = "\ninit: init process is running!\n";
    char* bufChildSuccess = "\nSuccessfully forked off child!\n";
    int status;

    write(1, bufWelcome, strlen(bufWelcome));

    if(fork() == 0){
        write(1, bufChildSuccess, strlen(bufChildSuccess));

        char* argv[] = {"./bin/test.elf", NULL};
        char* envp[] = {NULL};
        execve("bin/test.elf",argv,envp);
        exit(0);
    }

    for(;;){
        waitpid(-1, &status, 0);
    }
}