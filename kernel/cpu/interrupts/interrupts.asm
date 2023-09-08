;  
;   File: interrupts.asm
;
;   Author: Garnek
;   
;   Description: ASM stubs for interrupts
;
; SPDX-License-Identifier: BSD-2-Clause

extern exception_handler
extern irq_handler

bits 64

exception_common:
    ;push all general purpose registers
    push rax
    push rcx
    push rdx
    push rbx
    push rsp
    push rbp
    push rsi
    push rdi

    mov ax, ds
    push rax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov rdi, rsp
    ;clear direction flag
    cld

    call exception_handler

    pop rax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ;pop all general purpose registers
    pop rdi
    pop rsi
    pop rbp
    pop rsp
    pop rbx
    pop rdx
    pop rcx
    pop rax

    ;return and exclude some leftover bytes in the rsp
    add rsp, 16
    iretq

irq_common:
    push rax
    push rcx
    push rdx
    push rbx
    push rsp
    push rbp
    push rsi
    push rdi

    mov ax, ds
    push rax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov rdi, rsp
    cld

    call irq_handler

    pop rax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    pop rdi
    pop rsi
    pop rbp
    pop rsp
    pop rbx
    pop rdx
    pop rcx
    pop rax

    add rsp, 16
    iretq

global isr0
global isr1
global isr2
global isr3
global isr4
global isr5
global isr6
global isr7
global isr8
global isr9
global isr10
global isr11
global isr12
global isr13
global isr14
global isr15
global isr16
global isr17
global isr18
global isr19
global isr20
global isr21
global isr22
global isr23
global isr24
global isr25
global isr26
global isr27
global isr28
global isr29
global isr30
global isr31

global irq0
global irq1
global irq2
global irq3
global irq4
global irq5
global irq6
global irq7
global irq8
global irq9
global irq10
global irq11
global irq12
global irq13
global irq14
global irq15

; Divide Error
isr0:
    push byte 0
    push byte 0
    jmp exception_common

; Debug Exception
isr1:
    push byte 0
    push byte 1
    jmp exception_common

; Non Maskable Interrupt Exception
isr2:
    push byte 0
    push byte 2
    jmp exception_common

; Int 3 Exception
isr3:
    push byte 0
    push byte 3
    jmp exception_common

; INTO Exception
isr4:
    push byte 0
    push byte 4
    jmp exception_common

; Out of Bounds Exception
isr5:
    push byte 0
    push byte 5
    jmp exception_common

; Invalid Opcode Exception
isr6:
    push byte 0
    push byte 6
    jmp exception_common

; Coprocessor Not Available Exception
isr7:
    push byte 0
    push byte 7
    jmp exception_common

; Double Fault Exception
isr8:
    push byte 8
    jmp exception_common

; Coprocessor Segment Overrun Exception
isr9:
    push byte 0
    push byte 9
    jmp exception_common

; Bad TSS Exception
isr10:
    push byte 10
    jmp exception_common

; Segment Not Present Exception
isr11:
    push byte 11
    jmp exception_common

; Stack Fault Exception
isr12:
    push byte 12
    jmp exception_common

; General Protection Fault Exception
isr13:
    push byte 13
    jmp exception_common

; Page Fault Exception
isr14:
    push byte 14
    jmp exception_common

; Reserved Exception
isr15:
    push byte 0
    push byte 15
    jmp exception_common

; Floating Point Exception
isr16:
    push byte 0
    push byte 16
    jmp exception_common

; Alignment Check Exception
isr17:
    push byte 0
    push byte 17
    jmp exception_common

; Machine Check Exception
isr18:
    push byte 0
    push byte 18
    jmp exception_common

; Reserved
isr19:
    push byte 0
    push byte 19
    jmp exception_common

; Reserved
isr20:
    push byte 0
    push byte 20
    jmp exception_common

; Reserved
isr21:
    push byte 0
    push byte 21
    jmp exception_common

; Reserved
isr22:
    push byte 0
    push byte 22
    jmp exception_common

; Reserved
isr23:
    push byte 0
    push byte 23
    jmp exception_common

; Reserved
isr24:
    push byte 0
    push byte 24
    jmp exception_common

; Reserved
isr25:
    push byte 0
    push byte 25
    jmp exception_common

; Reserved
isr26:
    push byte 0
    push byte 26
    jmp exception_common

; Reserved
isr27:
    push byte 0
    push byte 27
    jmp exception_common

; Reserved
isr28:
    push byte 0
    push byte 28
    jmp exception_common

; Reserved
isr29:
    push byte 0
    push byte 29
    jmp exception_common

; Reserved
isr30:
    push byte 0
    push byte 30
    jmp exception_common

; Reserved
isr31:
    push byte 0
    push byte 31
    jmp exception_common

irq0:
    push byte 0
    push byte 32
    jmp irq_common

irq1:
    push byte 1
    push byte 33
    jmp irq_common

irq2:
    push byte 2
    push byte 34
    jmp irq_common

irq3:
    push byte 3
    push byte 35
    jmp irq_common

irq4:
    push byte 4
    push byte 36
    jmp irq_common

irq5:
    push byte 5
    push byte 37
    jmp irq_common

irq6:
    push byte 6
    push byte 38
    jmp irq_common

irq7:
    push byte 7
    push byte 39
    jmp irq_common

irq8:
    push byte 8
    push byte 40
    jmp irq_common

irq9:
    push byte 9
    push byte 41
    jmp irq_common

irq10:
    push byte 10
    push byte 42
    jmp irq_common

irq11:
    push byte 11
    push byte 43
    jmp irq_common

irq12:
    push byte 12
    push byte 44
    jmp irq_common

irq13:
    push byte 13
    push byte 45
    jmp irq_common

irq14:
    push byte 14
    push byte 46
    jmp irq_common

irq15:
    push byte 15
    push byte 47
    jmp irq_common
