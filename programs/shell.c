#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <stdbool.h>

#define MAX_PATH 4096
#define MAX_CMD 4096

char cmd[MAX_CMD];

void get_command(){
    int p = 0;
    int backspaces = 0;
    char chr;

    //flush stdin
    while(read(0, &chr, 1));
    chr = 0;

    while (chr != '\n')
    {
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

    char* argv[] = {0, 0};
    char* envp[] = {0};

    bool isDir = false;

    char* prog = mmap(NULL, MAX_PATH, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

    for(int i = 0; i < strlen(cmd); i++){
        if(cmd[i] == ' '){
            cmd[i] = 0;
            break;
        } else if(cmd[i] == '/') isDir = true;
    }

    memcpy(prog, cmd, strlen(cmd)+1);

    if(fork() == 0){
        if(!isDir){
            char* progBuf = mmap(NULL, MAX_PATH, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
            memcpy(progBuf, prog, strlen(prog)+1);
            memcpy(&prog[5], progBuf, strlen(progBuf)+1);
            munmap(progBuf, MAX_PATH);

            memcpy(prog, "/bin/", 5);

            if(strncmp(&prog[strlen(prog)-4], ".elf", 4)){
                memcpy(&prog[strlen(prog)], ".elf", 5);
            }
        }
        argv[0] = prog;
        int err = execve(prog, argv, envp);
        write(2, cmd, strlen(cmd));
        write(2, ": ", 2);
        switch(err){
            case -2: //ENOENT
                write(2, "Command not found.\n", 19);
                break;
            default:
                write(2, "Unknown error.\n", 15);
                break;
        }
        exit(-1);
    } else {
        int status;
        waitpid(-1, &status, 0);
    }
    munmap(prog, MAX_PATH);
}

void _start(){
    char* welcomeStr = "GarnOS Shell. Welcome to Userspace!\n\n";
    char cwd[MAX_PATH];

    write(1, welcomeStr, strlen(welcomeStr));

    for(;;){
        getcwd(cwd, MAX_PATH);
        write(1, cwd, strlen(cwd));
        write(1, ">", 1);

        get_command();

        if(cmd[0] == 0) continue;

        if(!strncmp("cd ", cmd, 3) || !strcmp("cd", cmd)){
            if(strlen(cmd) <= 3) continue;
            int status = chdir((cmd+3));

            if(status != 0){

                write(2, "shell: cd: ", 11);
                write(2, &cmd[3], strlen(&cmd[3]));
                write(2, ": ", 2);
                switch(status){
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
        } else {
            run_program();
        }
    }
}