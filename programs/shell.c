/*  
*   Program: shell
*
*   File: shell.c 
*
*   Program Author: Garnek
*   
*   Program Description: GarnOS's (default) shell.
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <stdio.h>
#include <unistd.h>
#include <string.h>
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

    //TODO: use scanf 
    //FIXME: scanf causes the shell to go crazy

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
    char* argv[MAX_ARGS+1] = {0};
    char* envp[] = {0};

    bool progString = true;

    int argc = 1;

    char* prog;
    int progSize;

    char* currentToken;

    currentToken = strtok(cmd, " ");

    if(!currentToken) return;

    while(currentToken){
        if(progString){
            prog = malloc(MAX_PATH);

            memcpy(prog, currentToken, strlen(currentToken)+1);

            if(currentToken[0] == '/' || (currentToken[0] == '.' && currentToken[1] == '/')){
                argv[0] = prog;
            } else {
                char* progBuf = malloc(MAX_PATH);
                memcpy(progBuf, prog, strlen(prog)+1);
                memcpy(&prog[5], progBuf, strlen(progBuf)+1);
                free(progBuf);

                memcpy(prog, "/zbin/", 5);

                if(strncmp(&prog[strlen(prog)-4], ".elf", 4)){
                    memcpy(&prog[strlen(prog)], ".elf", 5);
                }
                argv[0] = prog;
            }
            progString = false;
            goto gettoken;
        }

        argv[argc] = malloc(strlen(currentToken)+1);
        memcpy(argv[argc], currentToken, strlen(currentToken)+1);
        argc++;

gettoken:

        currentToken = strtok(NULL, " ");
    }

    if(fork() == 0){
        int err = execve(prog, argv, envp);
        fprintf(stderr, "%s: ", prog);
        perror(NULL);
        exit(-1);
    } else {
        int status;
        waitpid(-1, &status, 0);

        for(int i = 0; i < argc; i++){
            free(argv[i]);
        }
    }
}

int main(){
    char cwd[MAX_PATH];

    printf("GarnOS Shell. Welcome to Userspace!\n\n");

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