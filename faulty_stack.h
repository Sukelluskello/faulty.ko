#ifndef __FAULTY_STACK_H
#define __FAULTY_STACK_H

#include <linux/fs.h>

int init_stack_buffer_overflow(struct dentry* dir, const char* fn);
#endif
