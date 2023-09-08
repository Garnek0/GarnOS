/*  
*   File: term.h
*
*   Author: Garnek
*   
*   Description: Terminal Context structure definition
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef TERM_H
#define TERM_H

#include <types.h>

#include <term/font/font.h>
#include <term/cursor/cursor.h>

#define GLYPH_X 8
#define GLYPH_Y 16

#define FOREGROUND 0
#define BACKGROUND 1

#define TERM_VERSION "v0.1dev"

typedef struct {
    bool escape;
    bool escapeCSI;
    uint16_t escapeOffset;
    uint8_t escArgs[16];
    uint8_t escArgsCount;
    uint32_t foregroundColour; 
    uint32_t backgroundColour;
    cursor_t cursor;
} term_context_t;
extern term_context_t tc;

void term_init();
char term_putchar(char chr);
void term_scroll(uint16_t pix);
int term_print(char* str);
uint32_t term_get_colour(bool foreback);
void term_set_colour(uint32_t colour, bool foreback);

#endif //TERM_H