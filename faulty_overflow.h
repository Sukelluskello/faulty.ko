#ifndef __FAULTY_OVERFLOW_H
#define __FAULTY_OVERFLOW_H

#include <linux/fs.h>

int init_overflow(struct dentry* dir, const char* fn);
#endif
