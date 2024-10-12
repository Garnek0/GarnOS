#ifndef UNAME_H
#define UNAME_H

#include <garn/types.h>
#include <garn/irq.h>

#define UNAME_SYS_NAME "Garn"
#define UNAME_DEFAULT_NODE_NAME "Host"
#define UNAME_MACHINE_X86_64 "x86_64"

typedef struct _utsname {
    char sysName[65];
	char nodeName[65];
	char release[65];
	char version[65];
	char machine[65];
	char domainname[65];
} utsname_t;

#endif //UNAME_H
