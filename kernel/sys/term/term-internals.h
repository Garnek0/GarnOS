#pragma once

#include <sys/term/font/font.h>
#include <garn/fal/vfs.h>

void input_init();
ssize_t input_rb_read(size_t size, void* buf);

void term_init();
ssize_t term_read_as_file(vnode_t* self, size_t size, void* buf, size_t offset);
ssize_t term_write_as_file(vnode_t* self, size_t size, void* buf, size_t offset);


static vnode_operations_t termVnodeOps = {
	.vn_read = term_read_as_file,
	.vn_write = term_write_as_file,
	.vn_ioctl = NULL,
	.vn_inactive = NULL,
	.vn_lookup = NULL,
	.vn_mkdir = NULL,
	.vn_rmdir = NULL, 
	.vn_readdir = NULL
};
