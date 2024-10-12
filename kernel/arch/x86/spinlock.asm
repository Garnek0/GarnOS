;  
;   File: spinlock.asm
;
;   Author: Garnek
;   
;   Description: x86 spinlock implementation for safe SMP resource access
;
; SPDX-License-Identifier: BSD-2-Clause

bits 64

global acquireLock, releaseLock

acquireLock:
.retry:
    lock bts qword [rdi],0
    jc .retry
    ret
 
 
releaseLock:
    lock btr qword [rdi],0
    ret