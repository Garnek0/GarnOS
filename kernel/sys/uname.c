#include "uname-internals.h"

#include <garn/uname.h>
#include <garn/kernel.h>
#include <errno.h>
#include <garn/fal/vnode.h>
#include <garn/mm.h>

int sys_uname(stack_frame_t* regs, utsname_t* buf){
    if(!buf) return -EINVAL;

    memcpy(buf->sysName, UNAME_SYS_NAME, strlen(UNAME_SYS_NAME)+1);
    memcpy(buf->nodeName, UNAME_DEFAULT_NODE_NAME, strlen(UNAME_DEFAULT_NODE_NAME)+1);
    memcpy(buf->release, KERNEL_VER, strlen(KERNEL_VER)+1);

	vnode_t* file = vnode_open("0:/release", O_RDONLY, 0);
    if(!file){
        memcpy(buf->version, "unknown", 8);
    } else {
        int status = vnode_read(file, 64, buf->version, 0);
        if(status < 0) memcpy(buf->version, "unknown", 8);

        buf->version[status-1] = 0;

        vnode_close(file);
    }
    
    memcpy(buf->machine, UNAME_MACHINE_X86_64, strlen(UNAME_MACHINE_X86_64)+1);
    memcpy(buf->domainname, UNAME_DEFAULT_NODE_NAME, strlen(UNAME_DEFAULT_NODE_NAME)+1);
    return 0;
}
