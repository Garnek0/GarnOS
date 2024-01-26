#include <unistd.h>
#include <string.h>

void _start(){
    char* bufWelcome = "\ntest: New process running! You can type stuff!\n";

    write(1, bufWelcome, strlen(bufWelcome));

    char key;

    //flush stdin
    while(read(0, &key, 1));
    key = 0;

    for(;;){
        read(0, &key, 1);
        write(1, &key, 1);
        key = 0;
    }
}