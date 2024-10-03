/*  
*   Program: fib
*
*   File: fib.c
*
*   Program Author: Garnek
*   
*   Program Description: A simple test program. Prints Fibonacci numbers. 
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char** argv){
    if(argc == 1){
        printf("fib: Too few arguments.\n");
        return -1;
    } else if (argc >= 3){
        printf("fib: Too many arguments.\n");
        return -1;
    }

    int n = atoi(argv[1]);

    if(n > 93 || n <= 0){
        printf("fib: 1 <= n <= 93\n");
        return -1;
    }

    long long fib[n];

    if(n == 1){
        printf("Test Program. Here is the first Fibonacci number:\n1\n");
        return 0;
    } else if(n == 2){
        printf("Test Program. Here are the first 2 Fibonacci numbers:\n1 1\n");
        return 0;
    } else {
        printf("Test Program. Here are the first %d Fibonacci numbers:\n", n);

        fib[0] = 0;
        fib[1] = 1;

        for(int i = 2; i < n; i++){
            fib[i] = fib[i-1] + fib[i-2];
        }

        for(int i = 0; i < n; i++){
            printf("%lld ", fib[i]);
        }

        printf("\n");

        return 0;
    }

    
}
