#include "faulty_overflow.h"

#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/slab.h>

#define BUF_SIZE 256

static u8 counter = 250;
static struct dentry *fil;

static ssize_t overflow_read(struct file *fps, char *buf, size_t len, loff_t *offset);
static void overflow_non_reachable_function(void);

static const struct file_operations fops_overflow = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = overflow_read,
};

int init_overflow(struct dentry* dir, const char* fn) {
	fil = debugfs_create_file(fn, 0644, dir, NULL, &fops_overflow);
	if (fil == NULL) {
		pr_err("Faulty: Cannot create endpoint %s\n", fn);
		return -ENOENT;
	}

	// TODO signed underflow

	return 0;
}

static ssize_t overflow_read(struct file *fps, char __user *buf, size_t len,
			loff_t *offset)
{
	char *buffer = kmalloc(BUF_SIZE, GFP_KERNEL);
	ssize_t n = 0;

	snprintf(buffer, BUF_SIZE, "Faulty: Overflow - Counter value :%d\n", counter++); // note the behaviour of counter

	if (counter == 1)
		overflow_non_reachable_function();

	n =  simple_read_from_buffer(buf, len, offset, buffer,
				       strlen(buffer));
	kfree(buffer);
	return n;
}

static void overflow_non_reachable_function()
{
    pr_info("Faulty: Overflow - This function should not be reachable.\n");
}
