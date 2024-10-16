#include <garn/term/cursor.h>

#include <garn/term/term.h>
#include <garn/fb.h>

void cursor_advance(cursor_t* cursor){
    if(cursor->posX+GLYPH_X >= framebuffer_info.width){
        if(cursor->posY+GLYPH_Y > framebuffer_info.height-GLYPH_Y){
            term_scroll(GLYPH_Y);
            cursor->posX = 0;         
            return;
        }
        cursor->posY+=GLYPH_Y;
        cursor->posX = 0;
        return;
    }
    cursor->posX+=GLYPH_X;
}

void cursor_set(cursor_t* cursor, uint32_t x, uint32_t y){
    cursor->posX = x;
    cursor->posY = y;
}

void cursor_backspace(cursor_t* cursor){
    if((int32_t)cursor->posX - GLYPH_X >= 0){
        cursor->posX -= GLYPH_X;
        term_char(' ');
    } else {
        cursor->posX = framebuffer_info.width;
        cursor->posY -= GLYPH_Y;
        cursor->posX -= GLYPH_X;
        term_char(' ');
        cursor->posX = framebuffer_info.width;
        cursor->posY -= GLYPH_Y;
        cursor->posX -= GLYPH_X;
    }
}

void cursor_newline(cursor_t* cursor){
    if(cursor->posY+GLYPH_Y > framebuffer_info.height-GLYPH_Y){
        term_scroll(GLYPH_Y);
        cursor->posX = 0;
        //cursor->posY = framebuffer_info.heigth-GLYPH_Y;            
        return;
    }
    cursor->posY+=GLYPH_Y;
}

void cursor_return(cursor_t* cursor){
    cursor->posX = 0;
}
