#ifndef __FAULTY_SLAB_H
#define __FAULTY_SLAB_H

#include <linux/fs.h>

int init_slab_corruption(struct dentry* dir, const char* fn);
#endif
