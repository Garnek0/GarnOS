#include <unistd.h>

void _start(){
    
    write(1, "\e[2J", 4);
    
    exit(0);
}