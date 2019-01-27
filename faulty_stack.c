#include "faulty_stack.h"

#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <linux/fs.h>

struct dentry *fil;

static ssize_t sbo_read(struct file *fps, char *buf, size_t len, loff_t *offset);
static ssize_t sbo_write(struct file *fps, const char *buf, size_t len, loff_t *offset);
void non_reachable_function(void);

static char *buffer = "just some small data buffer";

struct dentry *fil;
int value;

static const struct file_operations fops_sbo = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = sbo_read,
	.write = sbo_write,
};

int init_stack_buffer_overflow(struct dentry *dir, const char *fn)
{
	fil = debugfs_create_file(fn, 0644, dir, &value, &fops_sbo);
	if (fil == NULL) {
		pr_err("Faulty: Cannot create endpoint %s\n", fn);
		return -ENOENT;
	}

	return 0;
}

static ssize_t sbo_read(struct file *fps, char *buf, size_t len,
			loff_t *offset)
{
	return simple_read_from_buffer(buf, len, offset, buffer,
				       strlen(buffer));

}

static ssize_t sbo_write(struct file *fps, const char *buf, size_t len,
			 loff_t *offset)
{
	int kbuf_size = 30;
	int flag = 0; // variable to clobber
	char kbuf[kbuf_size];
	char *kbuf_ptr = kbuf;
	char *pos = buffer;
	int bytes_written = 0;

	// Fault 1: length of the incoming data is used instead of
	// target buffer length
	bytes_written = simple_write_to_buffer(kbuf, len, offset,
					       buf, len);

	if (strlen(buffer) != bytes_written)
		return -EINVAL;

	while (*pos) {
		if (*pos++ != *kbuf_ptr++)
			return -EINVAL;
	}

	// we'll bypass stack canary evasion at this time
	if (flag != 0) {
		non_reachable_function();
	}

	return bytes_written;
}

void non_reachable_function(void)
{
	pr_info("Faulty: stack-buffer-overflow: this function should not be called!");
}
