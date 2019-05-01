/*
 * Faulty: A kernel module with intentional (and unintentional?) bugs
 */

#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define BUF_SIZE 256

static struct dentry *dir;
static const char *root = "ffaulty";

static int init_endpoint(struct dentry *dir, const char *fn,
			const struct file_operations *fops);
static ssize_t sbo_read(struct file *fps, __user char *buf, size_t len,
			loff_t *offset);
static ssize_t sbo_write(struct file *fps, const char __user *buf,
			size_t len, loff_t *offset);
static ssize_t slab_read(struct file *fps, char __user *buf, size_t len,
			loff_t *offset);
static ssize_t slab_write(struct file *fps, const char __user *buf,
			size_t len, loff_t *offset);
static void slab_operate_with_other_data(void);
static ssize_t unsigned_overflow_read(struct file *fps, char __user *buf,
				size_t len, loff_t *offset);
static ssize_t signed_underflow_read(struct file *fps, char __user *buf,
				size_t len, loff_t *offset);
static ssize_t format_read(struct file *fps, char __user *buf, size_t len,
			loff_t *offset);
static ssize_t format_write(struct file *fps, const char __user *buf,
			size_t len, loff_t *offset);
static ssize_t race_read(struct file *fps, char __user *buf, size_t len,
			loff_t *offset);
static ssize_t race_write(struct file *fps, const char __user *buf,
			size_t len, loff_t *offset);
static ssize_t df_alloc(struct file *fps, char __user *buf, size_t len,
			loff_t *offset);
static ssize_t df_free(struct file *fps, const char __user *buf,
		size_t len, loff_t *offset);
static ssize_t use_after_free_read(struct file *fps, char __user *buf,
				size_t len, loff_t *offset);
static ssize_t infoleak_read(struct file *fps, char __user *buf,
			size_t len, loff_t *offset);
static void non_reachable_function(void);

// stack buffer overflow
static char *buffer = "Write more than 10 bytes here to cause "
	"stack buffer overflow.\n";

static const struct file_operations fops_sbo = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = sbo_read,
	.write = sbo_write,
};

// slab corruption
static const struct file_operations fops_slab = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = slab_read,
	.write = slab_write,
};

struct some_data {
	char data[10];
	bool flag_which_is_never_set;
};

static struct some_data *user_controlled;
static struct some_data *other_data;
static bool toggle;

// under/overflow
static u8 unsigned_counter = 250;
static s8 signed_counter = -124;

static const struct file_operations fops_overflow = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = unsigned_overflow_read,
};

static const struct file_operations fops_underflow = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = signed_underflow_read,
};

// format string bug
static char *some_string = "A write to this endpoint will get copied "
	"to kernel message buffer\n";

static const struct file_operations fops_format = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = format_read,
	.write = format_write,
};

// data race
static char *race1;
static char *race2;

static const struct file_operations fops_race = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = race_read,
	.write = race_write,
};

// double free
static char *double_free;

static const struct file_operations fops_double_free = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = df_alloc,
	.write = df_free,
};

// use after free
static const struct file_operations fops_use_after_free = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = use_after_free_read,
};

static const struct file_operations fops_infoleak = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = infoleak_read,
};

// FAULT: infoleak
#define DATA_LEN 4096
struct a_struct {
	char data[DATA_LEN];
} *uninitialized;

static int __init mod_init(void)
{
	pr_debug("Faulty: creating debugfs-endpoints\n");

	dir = debugfs_create_dir(root, NULL);

	if (dir == ERR_PTR(-ENODEV)) {
		pr_err
		    ("Faulty: Debugfs doesn't seem to be compiled "
			    "into the kernel\n");
		return -ENODEV;
	}

	if (dir == NULL) {
		pr_err
		    ("Faulty: Cannot create debugfs-entry '%s'", root);
		return -ENOENT;
	}

	if (!init_endpoint(dir, "sbo", &fops_sbo))
		pr_debug
		    ("Faulty: Stack buffer overflow at debugfs '%s/sbo'\n",
			    root);

	if (!init_endpoint(dir, "slab", &fops_slab))
		pr_debug("Faulty: Slab buffer overflow at debugfs '%s/slab'\n",
			root);

	if (!init_endpoint(dir, "overflow", &fops_overflow))
		pr_debug("Faulty: Unsigned integer overflow at debugfs "
			"'%s/overflow'\n", root);

	if (!init_endpoint(dir, "underflow", &fops_underflow))
		pr_debug("Faulty: Signed integer underflow at debugfs "
			"'%s/underflow'\n", root);

	if (!init_endpoint(dir, "format", &fops_format))
		pr_debug("Faulty: Format string bug at debugfs "
			"'%s/format'\n", root);

	if (!init_endpoint(dir, "data-race", &fops_race)) {
		race1 = kzalloc(PAGE_SIZE, GFP_KERNEL);
		race2 = kzalloc(PAGE_SIZE, GFP_KERNEL);
		pr_debug("Faulty: Data race at debugfs "
			"'%s/data-race'\n", root);
	}

	if (!init_endpoint(dir, "double-free", &fops_double_free))
		pr_debug("Faulty: Double free bug at debugfs "
			"'%s/double-free'\n", root);

	if (!init_endpoint(dir, "use-after-free", &fops_use_after_free))
		pr_debug("Faulty: Use-after-free bug at debugfs "
			"'%s/use-after-free'\n", root);

	uninitialized = kmalloc(sizeof(struct a_struct), GFP_KERNEL);

	if (!init_endpoint(dir, "infoleak", &fops_infoleak))
		pr_debug("Faulty: Infoleak at debugfs '%s/infoleak'\n", root);

	pr_debug("Faulty: module loaded\n");
	return 0;

}

static void __exit mod_exit(void)
{
	debugfs_remove_recursive(dir);
	kfree(race1);
	kfree(race2);
	kfree(uninitialized);

	pr_debug("Faulty: Unloaded faulty kernel module\n");
}

static int init_endpoint(struct dentry *dir, const char *fn,
			const struct file_operations *fops)
{
	struct dentry *fil = debugfs_create_file(fn, 0644, dir, NULL, fops);

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

	// FAULT: stack buffer overflow
	// length of the incoming data is used instead of
	// target buffer length (kbuf_size)
	bytes_written = simple_write_to_buffer(kbuf, len, offset,
					       buf, len);

	// TODO: another fault here?
	//strncpy(buffer, kbuf, len);

	// we'll bypass stack canary evasion at this time
	if (flag != 0)
		non_reachable_function();

	return bytes_written;
}


static ssize_t slab_read(struct file *fps, char __user *buf, size_t len,
			loff_t *offset)
{
	slab_operate_with_other_data();

	if (!user_controlled) {
		pr_debug("Faulty: Slab - Read, no data\n");
		return 0;
	}

	pr_info("Faulty: Slab - Read, there is data\n");
	return simple_read_from_buffer(buf, len, offset,
				user_controlled->data,
				strlen(user_controlled->data));

}

static ssize_t slab_write(struct file *fps, const char __user *buf,
			size_t len, loff_t *offset)
{
	slab_operate_with_other_data();

	if (!user_controlled) {
		pr_debug("Faulty: Slab - Write, No data\n");
	} else {
		pr_debug("Faulty: Slab - Write, Free old data\n");
		kfree(user_controlled);
	}
	user_controlled = kmalloc(sizeof(struct some_data), GFP_KERNEL);

	// TODO test conditions
	if (other_data->flag_which_is_never_set)
		non_reachable_function();

	// FAULT: heap buffer overflow
	return simple_write_to_buffer(user_controlled->data, len, offset,
				buf, len);

}

// TODO: make this double freeable
static void slab_operate_with_other_data(void)
{
	if (!toggle) {
		toggle = true;
		pr_debug("Faulty: Slab - allocating other data");
		other_data = kzalloc(sizeof(struct some_data), GFP_KERNEL);
	} else {
		pr_debug("Faulty: Slab - freeing other data");
		kfree(other_data);
		toggle = false;
	}
}

static ssize_t unsigned_overflow_read(struct file *fps, char __user *buf,
				size_t len, loff_t *offset)
{
	char *buffer = kmalloc(BUF_SIZE, GFP_KERNEL);
	ssize_t n = 0;

	// FAULT: unsigned overflow
	snprintf(buffer, BUF_SIZE, "Faulty: Overflow - Counter value :%d\n",
		unsigned_counter++); // note the behaviour of counter

	if (unsigned_counter == 1)
		non_reachable_function();

	n =  simple_read_from_buffer(buf, len, offset, buffer,
				       strlen(buffer));
	kfree(buffer);
	return n;
}

static ssize_t signed_underflow_read(struct file *fps, char __user *buf,
				size_t len, loff_t *offset)
{
	char *buffer = kmalloc(BUF_SIZE, GFP_KERNEL);
	ssize_t n = 0;

	// FAULT: signed underflow
	snprintf(buffer, BUF_SIZE, "Faulty: Underflow - Counter value :%d\n",
		signed_counter--); // note the behaviour of counter

	if (signed_counter == 126)
		non_reachable_function();

	n =  simple_read_from_buffer(buf, len, offset, buffer,
				       strlen(buffer));
	kfree(buffer);
	return n;
}

static ssize_t format_read(struct file *fps, char __user *buf, size_t len,
			loff_t *offset)
{
	return simple_read_from_buffer(buf, len, offset, some_string,
				       strlen(some_string));
}

static ssize_t format_write(struct file *fps, const char __user *buf,
			size_t len, loff_t *offset)
{
	char buffer[BUF_SIZE];
	ssize_t n;

	n = simple_write_to_buffer(&buffer, BUF_SIZE, offset, buf, len);
	buffer[n] = '\0';
	pr_info("Faulty: %s\n", buffer);
	// pr_info(buffer); // this would generate a compile-time error
	// FAULT: format-string
	printk(buffer);
	return n;
}

static ssize_t race_read(struct file *fps, char __user *buf, size_t len,
			loff_t *offset)
{
	if (strcmp(race1, race2))
		non_reachable_function();

	return simple_read_from_buffer(buf, len, offset, race1,
				strlen(race1));
}

static ssize_t race_write(struct file *fps, const char __user *buf,
			size_t len, loff_t *offset)
{
	// FAULT: stack overflow
	char buffer[16 * PAGE_SIZE];
	ssize_t n;

	n = simple_write_to_buffer(&buffer, PAGE_SIZE, offset, buf, len);
	buffer[n] = '\0';

	// FAULT: race
	// slow write is racy
	memcpy(race1, buffer, len);
	udelay(1000);
	memcpy(race2, buffer, len);

	return n;
}

static ssize_t df_alloc(struct file *fps, char __user *buf,
			size_t len, loff_t *offset)
{
	pr_info("Faulty: double-free allocation\n");
	double_free = kmalloc(len, GFP_KERNEL);
	return len;
}
static ssize_t df_free(struct file *fps, const char __user *buf,
		size_t len, loff_t *offset)
{
	// FAULT: double free
	pr_info("Faulty: double-free deallocation\n");
	kfree(double_free);
	return len;
}

static ssize_t use_after_free_read(struct file *fps, char __user *buf,
				size_t len, loff_t *offset)
{
	char *tmp = kmalloc(len, GFP_KERNEL);

	strncpy(tmp, buffer, len);
	// FAULT: use after free
	kfree(tmp);
	copy_to_user(buf, tmp, len);
	return len;
}


static ssize_t infoleak_read(struct file *fps, char __user *buf,
			size_t len, loff_t *offset)
{

	ssize_t l = len < DATA_LEN ? len : DATA_LEN;

	return simple_read_from_buffer(buf, len, offset,
				uninitialized->data, l);

}

static void non_reachable_function(void)
{
	pr_info("Faulty: This function should not be reachable.\n");
}

module_init(mod_init);
module_exit(mod_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A Kernel Module with Faults");
MODULE_AUTHOR("Ilja Sidoroff");
