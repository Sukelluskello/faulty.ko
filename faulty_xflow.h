#ifndef __FAULTY_XFLOW_H
#define __FAULTY_XFLOW_H

#include <linux/fs.h>

int init_xflow(struct dentry* dir, const char* fn);
#endif
