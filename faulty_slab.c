#include "faulty_slab.h"

#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/slab.h>

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

struct user_controlled_data {
    char data[10];
    bool flag;
} *user_controlled = NULL;

struct unrelated_data {
    char data[10];
} *unrelated = NULL;


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
    if (!user_controlled) {
	pr_debug("Faulty: Slab - Read, no data\n");
	return 0;
    }

    pr_info("Faulty: Slab - Read, there is data\n");
    return simple_read_from_buffer(buf, len, offset,
			    user_controlled->data, strlen(user_controlled->data));

}

static ssize_t slab_write(struct file *fps, const char *buf, size_t len,
			 loff_t *offset)
{
    // TODO: should have also try to have double free here
    if (!user_controlled) {
	pr_debug("Faulty: Slab - Write, No data\n");
    } else {
	pr_debug("Faulty: Slab - Write, Free old data\n");
	kfree(user_controlled);
    }
    user_controlled = kmalloc(sizeof (struct user_controlled_data), GFP_KERNEL);
    return simple_write_to_buffer(user_controlled->data, len, offset,
				  buf, len);

}

void slab_non_reachable_function(void)
{
    pr_info("This function should not be reachable.\n");
}
