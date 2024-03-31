#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>

#define MAX_PATH 4096
#define MAX_CMD 4096
#define MAX_ARGS 256

char cmd[MAX_CMD];

void get_command(){
    int p = 0;
    int backspaces = 0;
    char chr;

    //flush stdin
    while(read(0, &chr, 1));
    chr = 0;

    while (chr != '\n'){
        chr = 0;
        read(0, &chr, 1);
        if(chr == 0) continue;

        if(chr == '\b'){
            if(backspaces){
                backspaces--;
                write(1, "\b", 1);
                
                p--;
            }
            continue;
        }
        write(1, &chr, 1);
        cmd[p] = chr;
        backspaces++;
        p++;
        if(p > 4096) return;
    }
    cmd[--p] = 0;
}

void run_program(){
    //TODO: Add support for arguments

    char* argv[MAX_ARGS+1] = {0};
    char* envp[] = {0};

    bool isDir = false;
    bool progString = true;
    bool leadingSpace = true;
    int argc = 1;

    char* prog = malloc(MAX_PATH);
    int progSize;

    for(int i = 0; i < strlen(cmd); i++){
        //TODO: Add support for stuff like pipes
        if(cmd[i] == ' '){
            if(leadingSpace){
                memcpy(cmd, cmd+1, strlen(cmd+1)+1);
                i--;
                continue;
            } else if(cmd[i+1] == ' ' || cmd[i+1] == '\n' || cmd[i+1] == 0){
                memcpy(&cmd[i], &cmd[i+1], strlen(&cmd[i+1])+1);
                i--;
                continue;
            }

            if(progString){
                progString = false;
                progSize = i;
                memcpy(prog, cmd, progSize);
                prog[progSize] = 0;
            }
            int istart = i+1;
            while(cmd[i+1] != ' ' && cmd[i+1] != '\n' && cmd[i+1] != 0) i++;
            int iend = i;

            argv[argc] = malloc(iend-istart+2);
            (argv[argc])[iend-istart+1] = 0;
            memcpy(argv[argc], &cmd[istart], iend-istart+1);
            argc++;
        } else if(cmd[i] == '/' && progString) isDir = true;
        else {
            leadingSpace = false;
        }
    }

    if(progString) memcpy(prog, cmd, strlen(cmd)+1);

    if(fork() == 0){
        if(!isDir){
            char* progBuf = malloc(MAX_PATH);
            memcpy(progBuf, prog, strlen(prog)+1);
            memcpy(&prog[5], progBuf, strlen(progBuf)+1);
            free(progBuf);

            memcpy(prog, "/bin/", 5);

            if(strncmp(&prog[strlen(prog)-4], ".elf", 4)){
                memcpy(&prog[strlen(prog)], ".elf", 5);
            }
        }
        argv[0] = prog;
        int err = execve(prog, argv, envp);
        fprintf(stderr, "%s: ", prog);
        perror(NULL);
        exit(-1);
    } else {
        int status;
        waitpid(-1, &status, 0);
    }
    for(int i = 0; i < argc; i++){
        free(argv[i]);
    }
}

int main(){
    char cwd[MAX_PATH];

    printf("GarnOS Shell. Welcome to Userspace!\n\n");

    opendir("/bin");

    for(;;){
        getcwd(cwd, MAX_PATH);
        printf("%s>", cwd);

        get_command();

        if(cmd[0] == 0) continue;

        if(!strncmp("cd ", cmd, 3) || !strcmp("cd", cmd)){
            if(strlen(cmd) <= 3) continue;
            int status = chdir((cmd+3));

            if(status != 0){
                fprintf(stderr, "shell: cd: %s: ", &cmd[3]);
                perror(NULL);
            }
        } else {
            run_program();
        }
    }
}