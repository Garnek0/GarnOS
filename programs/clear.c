/*  
*   Program: clear
*
*   File: clear.c 
*
*   Program Author: Garnek
*   
*   Program Description: Clears screen.
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <stdio.h>

int main(){
    printf("\e[2J");
}
