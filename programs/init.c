
void _start(){
    asm volatile("mov $0xDEADBEEF, %rax");
    for(;;);
}