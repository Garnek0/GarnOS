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
extern syscall_handler

bits 64

exception_common:
    ;push all general purpose registers
    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rax, ds
    push rax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov rdi, rsp
    ;clear direction flag
    cld

    call exception_handler

    pop rax
    mov ds, ax
    mov es, ax
    
    ;pop all general purpose registers
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
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
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rax, ds
    push rax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov rdi, rsp
    cld

    call irq_handler

    pop rax
    mov ds, ax
    mov es, ax

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    pop rax

    add rsp, 16
    iretq

syscall_common:
    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rax, ds
    push rax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov rdi, rsp
    cld

    call syscall_handler

    pop rax
    mov ds, ax
    mov es, ax

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
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

global isr128

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
global irq223
global irq224

; Divide Error
isr0:
    push 0
    push 0
    jmp exception_common

; Debug Exception
isr1:
    push 0
    push 1
    jmp exception_common

; Non Maskable Interrupt Exception
isr2:
    push 0
    push 2
    jmp exception_common

; Int 3 Exception
isr3:
    push 0
    push 3
    jmp exception_common

; INTO Exception
isr4:
    push 0
    push 4
    jmp exception_common

; Out of Bounds Exception
isr5:
    push 0
    push 5
    jmp exception_common

; Invalid Opcode Exception
isr6:
    push 0
    push 6
    jmp exception_common

; Coprocessor Not Available Exception
isr7:
    push 0
    push 7
    jmp exception_common

; Double Fault Exception
isr8:
    push 8
    jmp exception_common

; Coprocessor Segment Overrun Exception
isr9:
    push 0
    push 9
    jmp exception_common

; Bad TSS Exception
isr10:
    push 10
    jmp exception_common

; Segment Not Present Exception
isr11:
    push 11
    jmp exception_common

; Stack Fault Exception
isr12:
    push 12
    jmp exception_common

; General Protection Fault Exception
isr13:
    push 13
    jmp exception_common

; Page Fault Exception
isr14:
    push 14
    jmp exception_common

; Reserved Exception
isr15:
    push 0
    push 15
    jmp exception_common

; Floating Point Exception
isr16:
    push 0
    push 16
    jmp exception_common

; Alignment Check Exception
isr17:
    push 0
    push 17
    jmp exception_common

; Machine Check Exception
isr18:
    push 0
    push 18
    jmp exception_common

; Reserved
isr19:
    push 0
    push 19
    jmp exception_common

; Reserved
isr20:
    push 0
    push 20
    jmp exception_common

; Reserved
isr21:
    push 0
    push 21
    jmp exception_common

; Reserved
isr22:
    push 0
    push 22
    jmp exception_common

; Reserved
isr23:
    push 0
    push 23
    jmp exception_common

; Reserved
isr24:
    push 0
    push 24
    jmp exception_common

; Reserved
isr25:
    push 0
    push 25
    jmp exception_common

; Reserved
isr26:
    push 0
    push 26
    jmp exception_common

; Reserved
isr27:
    push 0
    push 27
    jmp exception_common

; Reserved
isr28:
    push 0
    push 28
    jmp exception_common

; Reserved
isr29:
    push 0
    push 29
    jmp exception_common

; Reserved
isr30:
    push 0
    push 30
    jmp exception_common

; Reserved
isr31:
    push 0
    push 31
    jmp exception_common

; Syscall interrupt
isr128:
    push 0
    push 0
    jmp syscall_common

irq0:
    push 0
    push 32
    jmp irq_common

irq1:
    push 1
    push 33
    jmp irq_common

irq2:
    push 2
    push 34
    jmp irq_common

irq3:
    push 3
    push 35
    jmp irq_common

irq4:
    push 4
    push 36
    jmp irq_common

irq5:
    push 5
    push 37
    jmp irq_common

irq6:
    push 6
    push 38
    jmp irq_common

irq7:
    push 7
    push 39
    jmp irq_common

irq8:
    push 8
    push 40
    jmp irq_common

irq9:
    push 9
    push 41
    jmp irq_common

irq10:
    push 10
    push 42
    jmp irq_common

irq11:
    push 11
    push 43
    jmp irq_common

irq12:
    push 12
    push 44
    jmp irq_common

irq13:
    push 13
    push 45
    jmp irq_common

irq14:
    push 14
    push 46
    jmp irq_common

irq15:
    push 15
    push 47
    jmp irq_common

irq223:
    push 223
    push 254
    jmp irq_common

irq224:
    push 224
    push 255
    jmp irq_common
