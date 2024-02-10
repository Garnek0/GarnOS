#include <unistd.h>
#include <string.h>

void _start(){
    char* testStr = "Test Program. Here are the first 7 Fibonacci numbers:\n";

    write(1, testStr, strlen(testStr));

    char fibChar;
    int f1 = 0, f2 = 1, aux;
    fibChar = f1 + '0';
    write(1, &fibChar, 1);
    write(1, " ", 1);
    fibChar = f2 + '0';
    write(1, &fibChar, 1);
    write(1, " ", 1);

    for(int i = 0; i < 5; i++){
        aux = f1;
        f1 = f2;
        f2 = aux + f2;

        fibChar = f2 + '0';
        write(1, &fibChar, 1);
        write(1, " ", 1);
    }
    write(1, "\n", 1);

    exit(0);
}