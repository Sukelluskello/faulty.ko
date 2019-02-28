#ifndef __FAULTY_FORMAT_H
#define __FAULTY_FORMAT_H

#include <linux/fs.h>

int init_format(struct dentry* dir, const char* fn);
#endif
