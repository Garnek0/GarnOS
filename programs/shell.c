#include <unistd.h>
#include <string.h>

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

void _start(){
    char* welcomeStr = "GarnOS Shell. Welcome to Userspace!\n\n";
    char cwd[MAX_PATH];

    write(1, welcomeStr, strlen(welcomeStr));

    for(;;){
        getcwd(cwd, MAX_PATH);
        write(1, cwd, strlen(cwd));
        write(1, ">", 1);

        get_command();

        if(!strncmp("cd ", cmd, 3)){
            int status = chdir((cmd+3));
            if(status != 0){
                switch ((status))
                {
                case -2: //ENOENT
                    write(2, "cd: No such file or directory.\n", 31);
                    break;
                case -20: //ENOTDIR
                    write(2, "cd: Not a directory.\n", 21);
                    break;
                default:
                    write(2, "cd: Unknown error.\n", 19);
                    break;
                }
            }
        } else {
            write(1, "readback: ", 10);
            write(1, cmd, strlen(cmd));
            write(1, "\n", 1);
        }
    }
    char key;

    for(;;){
        read(0, &key, 1);
        write(1, &key, 1);
        key = 0;
    }
}