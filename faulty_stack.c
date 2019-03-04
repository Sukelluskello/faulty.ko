#include "faulty_stack.h"

#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <linux/fs.h>

static struct dentry *fil;

static ssize_t sbo_read(struct file *fps, char *buf, size_t len, loff_t *offset);
static ssize_t sbo_write(struct file *fps, const char *buf, size_t len, loff_t *offset);
void non_reachable_function(void);

static char *buffer = "just some small data buffer\n";

// TODO perhaps allocate some memory here to avoid clobbering fobs_sbo

static const struct file_operations fops_sbo = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = sbo_read,
	.write = sbo_write,
};

int init_stack_buffer_overflow(struct dentry *dir, const char *fn)
{
	fil = debugfs_create_file(fn, 0644, dir, NULL, &fops_sbo);
	if (fil == NULL) {
		pr_err("Faulty: Cannot create endpoint %s\n", fn);
		return -ENOENT;
	}

	return 0;
}

static ssize_t sbo_read(struct file *fps, char __user *buf, size_t len,
			loff_t *offset)
{
	return simple_read_from_buffer(buf, len, offset, buffer,
				       strlen(buffer));
}

static ssize_t sbo_write(struct file *fps, const char __user *buf, size_t len,
			 loff_t *offset)
{
	int kbuf_size = 10;
	int flag = 0; // variable to clobber
	char kbuf[kbuf_size];
	int bytes_written = 0;

	// Fault-SBO: length of the incoming data is used instead of
	// target buffer length (kbuf_size)
	bytes_written = simple_write_to_buffer(kbuf, len, offset,
					       buf, len);

	// TODO: another fault here?
	//strncpy(buffer, kbuf, len);

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
