#pragma once

#include <garn/types.h>
#include <garn/fal/vfs.h>

int devfs_init(const char* mount);

vnode_t* devfs_lookup(vnode_t* self, const char* name);
ssize_t devfs_readdir(vnode_t* self, size_t count, void* buf, size_t offset);
