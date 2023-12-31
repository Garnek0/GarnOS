
void _start(){
    asm volatile("mov $0, %rax\n"
                 "mov $1, %rdi\n"
                 "mov $2, %rsi\n"
                 "mov $3, %rdx\n"
                 "mov $4, %r10\n"
                 "mov $5, %r8\n"
                 "mov $6, %r9\n"
                 "int $0x80");
    for(;;);
}