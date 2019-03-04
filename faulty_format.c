#include "faulty_format.h"

#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <linux/fs.h>

#define BUF_SIZE 256

static struct dentry *fil;
static char *some_string = "A write to this endpoint will get copied to kernel message buffer\n";

static ssize_t format_read(struct file *fps, char *buf, size_t len, loff_t *offset);
static ssize_t format_write(struct file *fps, const char *buf, size_t len, loff_t *offset);

static const struct file_operations fops_format = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = format_read,
	.write = format_write,
};

int init_format(struct dentry* dir, const char* fn) {
	fil = debugfs_create_file(fn, 0644, dir, NULL, &fops_format);
	if (fil == NULL) {
		pr_err("Faulty: Cannot create endpoint %s\n", fn);
		return -ENOENT;
	}

	return 0;
}

static ssize_t format_read(struct file *fps, char __user *buf, size_t len,
			loff_t *offset)
{
	return simple_read_from_buffer(buf, len, offset, some_string,
				       strlen(some_string));
}

static ssize_t format_write(struct file *fps, const char __user *buf, size_t len,
			 loff_t *offset)
{
	char buffer[BUF_SIZE];
	ssize_t n = simple_write_to_buffer(&buffer, BUF_SIZE, offset, buf, len);
	buffer[n] = '\0';
	pr_info("Faulty: %s\n", buffer);
	// pr_info(buffer); // this would generate a compile-time error
	printk(buffer); // vulnerable
	return n;
}
