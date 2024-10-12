#ifndef TERM_H
#define TERM_H

#include <garn/types.h>

#include <garn/term/cursor.h>
#include <garn/spinlock.h>

#define GLYPH_X 8
#define GLYPH_Y 16

#define FOREGROUND 0
#define BACKGROUND 1

typedef struct {
    bool escape;
    bool escapeCSI;
    bool enabled;
    uint16_t escapeOffset;
    uint8_t escArgs[16];
    uint8_t escArgsCount;
    uint32_t foregroundColour; 
    uint32_t backgroundColour;
    cursor_t cursor;
    spinlock_t lock; //terminal emulator lock
} term_context_t;
extern term_context_t tc;

void term_clear();
void term_char(char chr);
char term_putchar(char chr);
char term_putchar_dbg(char chr);
void term_scroll(uint16_t pix);
int term_print(char* str);
int term_print_dbg(char* str);
uint32_t term_get_colour(bool foreback);
void term_set_colour(uint32_t colour, bool foreback);
void term_disable();
void term_enable();

#endif //TERM_H
