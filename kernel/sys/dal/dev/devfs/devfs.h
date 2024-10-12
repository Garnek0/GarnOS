#ifndef DEVFS_H
#define DEVFS_H

#include <garn/types.h>
#include <garn/fal/vfs.h>

int devfs_init(const char* mount);

vnode_t* devfs_lookup(vnode_t* self, const char* name);

#endif //DEVFS_H
