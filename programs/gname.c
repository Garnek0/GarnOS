/*  
*   Program: gname
*
*   File: gname.c
*
*   Program Author: Garnek
*   
*   Program Description: Displays Garn kernel name + other OS details
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <stdio.h>
#include <unistd.h>
#include <abi-bits/utsname.h>

int main(int argc, char** argv){
    struct utsname name;
    
    uname(&name);

    char opt;

    if(argc == 1){
        printf("%s\n", name.sysname);
        return 0;
    }

    while((opt = getopt(argc, argv, "snrvmpioa")) != -1){
        switch(opt){
            case 's':
                printf("%s\n", name.sysname);
                break;
            case 'n':
                printf("%s\n", name.nodename);
                break;
            case 'r':
                printf("%s\n", name.version);
                break;
            case 'v':
                printf("%s\n", name.release);
                break;
            case 'm':
            case 'p':
            case 'i':
                printf("%s\n", name.machine);
                break;
            case 'o':
                printf("GarnOS\n");
                break;
            case 'a':
                printf("%s %s %s %s %s %s\n", name.sysname, name.nodename, name.version, name.release, name.machine, "GarnOS");
                break;
            default:
                break;
        }
    }

    return 0;
}
