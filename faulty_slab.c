#include "faulty_slab.h"

#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/slab.h>

struct dentry *fil_slab;

static ssize_t slab_read(struct file *fps, char *buf, size_t len, loff_t *offset);
static ssize_t slab_write(struct file *fps, const char *buf, size_t len, loff_t *offset);
void operate_with_other_data(void);
void slab_non_reachable_function(void);

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

struct some_data *user_controlled = NULL;
struct some_data *other_data = NULL;
bool toggle = false;

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
    operate_with_other_data();
    
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
    operate_with_other_data();

    if (!user_controlled) {
	pr_debug("Faulty: Slab - Write, No data\n");
    } else {
	pr_debug("Faulty: Slab - Write, Free old data\n");
	kfree(user_controlled);
    }
    user_controlled = kmalloc(sizeof (struct some_data), GFP_KERNEL);

    // TODO test conditions
    if (other_data->flag_which_is_never_set)
	slab_non_reachable_function();
    
    return simple_write_to_buffer(user_controlled->data, len, offset,
				  buf, len);

}

// TODO: make this double freeable
void operate_with_other_data() {
    if (!toggle) {
	toggle = true;
	pr_debug("Faulty: Slab - allocating other data");
	other_data = kzalloc(sizeof (struct some_data), GFP_KERNEL);
    } else {
	pr_debug("Faulty: Slab - freeing other data");
	kfree(other_data);
	toggle = false;
    }
}
    
void slab_non_reachable_function(void)
{
    pr_info("Faulty: Slab - This function should not be reachable.\n");
}
