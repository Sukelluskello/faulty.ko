#include "faulty_slab.h"

#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <linux/fs.h>

struct dentry *fil_slab;

static ssize_t slab_read(struct file *fps, char *buf, size_t len, loff_t *offset);
static ssize_t slab_write(struct file *fps, const char *buf, size_t len, loff_t *offset);
void non_reachable_function(void);


static const struct file_operations fops_slab = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = slab_read,
	.write = slab_write,
};

// TODO: perhaps abstract
int init_slab_corruption(struct dentry *dir, const char *fn)
{
	fil_slab = debugfs_create_file(fn, 0644, dir, NULL, &fops_slab);
	if (fil_slab == NULL) {
		pr_err("Faulty: Cannot create endpoint %s\n", fn);
		return -ENOENT;
	}

	return 0;
}

static ssize_t slab_read(struct file *fps, char *buf, size_t len,
			loff_t *offset)
{
    return 0; // TODO implement me
}

static ssize_t slab_write(struct file *fps, const char *buf, size_t len,
			 loff_t *offset)
{
    return 0; // TODO implement me
}

void slab_non_reachable_function(void)
{

}
