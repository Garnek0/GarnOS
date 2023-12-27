
void _start(){
    asm volatile("mov $0xDEADBEEF, %rax; int $0x80");
    for(;;);
}