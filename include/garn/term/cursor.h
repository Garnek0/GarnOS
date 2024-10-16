#pragma once

#include <garn/types.h>

typedef struct {
    uint32_t posX;
    uint32_t posY;
} cursor_t;

void cursor_advance(cursor_t* cursor);
void cursor_set(cursor_t* cursor, uint32_t x, uint32_t y);
void cursor_newline(cursor_t* cursor);
void cursor_return(cursor_t* cursor);
void cursor_backspace(cursor_t* cursor);
