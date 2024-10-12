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
