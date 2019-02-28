#ifndef __FAULTY_RACE_H
#define __FAULTY_RACE_H

#include <linux/fs.h>

int init_data_race(struct dentry* dir, const char* fn);
#endif
