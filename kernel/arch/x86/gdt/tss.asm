;  
;   File: gdt.asm
;
;   Author: Garnek
;   
;   Description: TSS flushing
;
; SPDX-License-Identifier: BSD-2-Clause

global tss_flush

tss_flush:
    mov ax, 0x28
    ltr ax
    ret