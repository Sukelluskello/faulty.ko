/*
 * A kernel module with intentional (and unintentional?) bugs
 */

#include "faulty_slab.h"
#include "faulty_stack.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <linux/fs.h>

struct dentry *dir;

static int __init mod_init(void)
{
	pr_debug("Faulty: creating debugfs-endpoints\n");

	dir = debugfs_create_dir("faulty", NULL);

	if (dir == ERR_PTR(-ENODEV)) {
		pr_err
		    ("Faulty: Debugfs seems to be compiled into the kernel\n");
		return -ENODEV;
	}

	if (dir == NULL) {
		pr_err
		    ("Faulty: Cannot create debugfs-entry 'faulty'");
		return -ENOENT;
	}
	// Create endpoints for potential vulnerabilities
	if (!init_stack_buffer_overflow(dir, "sbo"))
		pr_debug
		    ("Faulty: Stack buffer overflow at debugfs 'faulty/sbo'\n");
	else
		pr_err
		    ("Faulty: Cannot create debugfs-entry faulty/sbo\n");

	if (!init_slab_corruption(dir, "slab"))
	    pr_err("Faulty: Cannot create debugfs-entry faulty/slab\n");

	pr_debug("Faulty: module loaded\n");
	return 0;

}

static void __exit mod_exit(void)
{
	debugfs_remove_recursive(dir);
	pr_debug("Faulty: Unloaded faulty kernel module\n");
}

module_init(mod_init);
module_exit(mod_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A Kernel Module with Faults");
MODULE_AUTHOR("Ilja Sidoroff");
