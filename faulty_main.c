/*
 * A kernel module with intentional (and unintentional?) bugs
 */

#include "faulty_format.h"
#include "faulty_race.h"
#include "faulty_slab.h"
#include "faulty_stack.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/slab.h>

#define BUF_SIZE 256

struct dentry *dir;
const char *root = "ffaulty";

static int init_endpoint(struct dentry* dir, const char* fn, const struct file_operations *fops);
static void non_reachable_function(void);

// under/overflow
static u8 unsigned_counter = 250;
static s8 signed_counter = -124;

//static struct dentry *fil_overflow;
//static struct dentry *fil_underflow;

static ssize_t unsigned_overflow_read(struct file *fps, char *buf, size_t len, loff_t *offset);

static const struct file_operations fops_overflow = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = unsigned_overflow_read,
};

static ssize_t signed_underflow_read(struct file *fps, char *buf, size_t len, loff_t *offset);

static const struct file_operations fops_underflow = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = signed_underflow_read,
};

static int __init mod_init(void)
{
	pr_debug("Faulty: creating debugfs-endpoints\n");

	dir = debugfs_create_dir(root, NULL);

	if (dir == ERR_PTR(-ENODEV)) {
		pr_err
		    ("Faulty: Debugfs doesn't seem to be compiled into the kernel\n");
		return -ENODEV;
	}

	if (dir == NULL) {
		pr_err
		    ("Faulty: Cannot create debugfs-entry '%s'", root);
		return -ENOENT;
	}

	if (!init_stack_buffer_overflow(dir, "sbo"))
		pr_debug
		    ("Faulty: Stack buffer overflow at debugfs '%s/sbo'\n", root);
	else
		pr_err
		    ("Faulty: Cannot create debugfs-entry %s/sbo\n", root);

	if (!init_slab_corruption(dir, "slab"))
	    pr_debug("Faulty: Slab buffer overflow at debugfs '%s/slab'\n", root);
	else
	    pr_err("Faulty: Cannot create debugfs-entry %s/slab\n", root);

	if (!init_data_race(dir, "data-race"))
	    pr_debug("Faulty: Data race at debugfs '%s/data-race'\n", root);
	else
	    pr_err("Faulty: Cannot create debugfs-entry %s/data-race\n", root);

	if (!init_endpoint(dir, "overflow", &fops_overflow))
	    pr_debug("Faulty: Unsigned integer overflow at debugfs '%s/overflow'\n", root);
	else
	    pr_err("Faulty: Cannot create debugfs-entry %s/overflow\n", root);

	if (!init_endpoint(dir, "underflow", &fops_underflow))
	    pr_debug("Faulty: Signed integer underflow at debugfs '%s/underflow'\n", root);
	else
	    pr_err("Faulty: Cannot create debugfs-entry %s/underflow\n", root);

	if (!init_format(dir, "format"))
	    pr_debug("Faulty: Format string bug at debugfs '%s/format'\n", root);
	else
	    pr_err("Faulty: Cannot create debugfs-entry %s/format\n", root);

	pr_debug("Faulty: module loaded\n");
	return 0;

}

static void __exit mod_exit(void)
{
	debugfs_remove_recursive(dir);
	pr_debug("Faulty: Unloaded faulty kernel module\n");
}

static int init_endpoint(struct dentry* dir, const char* fn, const struct file_operations *fops) {
	struct dentry *fil = debugfs_create_file(fn, 0644, dir, NULL, fops);
	if (fil == NULL) {
		pr_err("Faulty: Cannot create endpoint %s\n", fn);
		return -ENOENT;
	}

	return 0;
}

static ssize_t unsigned_overflow_read(struct file *fps, char __user *buf, size_t len,
			loff_t *offset)
{
	char *buffer = kmalloc(BUF_SIZE, GFP_KERNEL);
	ssize_t n = 0;

	snprintf(buffer, BUF_SIZE, "Faulty: Overflow - Counter value :%d\n",
		unsigned_counter++); // note the behaviour of counter

	if (unsigned_counter == 1)
		non_reachable_function();

	n =  simple_read_from_buffer(buf, len, offset, buffer,
				       strlen(buffer));
	kfree(buffer);
	return n;
}

static ssize_t signed_underflow_read(struct file *fps, char __user *buf, size_t len,
			loff_t *offset)
{
	char *buffer = kmalloc(BUF_SIZE, GFP_KERNEL);
	ssize_t n = 0;

	snprintf(buffer, BUF_SIZE, "Faulty: Underflow - Counter value :%d\n",
		signed_counter--); // note the behaviour of counter

	if (signed_counter == 126)
		non_reachable_function();

	n =  simple_read_from_buffer(buf, len, offset, buffer,
				       strlen(buffer));
	kfree(buffer);
	return n;
}

static void non_reachable_function()
{
    pr_info("Faulty: This function should not be reachable.\n");
}

module_init(mod_init);
module_exit(mod_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A Kernel Module with Faults");
MODULE_AUTHOR("Ilja Sidoroff");
