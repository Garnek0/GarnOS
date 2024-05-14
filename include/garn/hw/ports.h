#include <garn/types.h>

static inline void outb(uint16_t port, uint8_t data){
    asm volatile("outb %b0, %w1" : : "a" (data), "Nd" (port));
}

static inline uint8_t inb(uint16_t port){
    uint8_t data;
    asm volatile("inb %w1, %b0" : "=a" (data) : "Nd" (port));
    return data;
}

static inline void outw(uint16_t port, uint16_t data){
    asm volatile("outw %w0, %w1" : : "a" (data), "Nd" (port));
}

static inline uint16_t inw(uint16_t port){
    uint16_t data;
    asm volatile("inw %w1, %w0" : "=a" (data) : "Nd" (port));
    return data;
}

static inline void outl(uint16_t port, uint32_t data){
    asm volatile("outl %0, %w1" : : "a" (data), "Nd" (port));
}

static inline uint32_t inl(uint16_t port){
    uint32_t data;
    asm volatile("inl %w1, %0" : "=a" (data) : "Nd" (port));
    return data;
}

static inline void io_wait(){
    outb(0x80, 0);
}