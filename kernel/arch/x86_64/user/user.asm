global user_jump

user_jump:
    mov ax, 0x23
    mov ds, ax

    push 0x23
    push rsi
    push 0x200
    push 0x1b
    push rdi
    iretq
