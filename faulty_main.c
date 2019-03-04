/*
 * A kernel module with intentional (and unintentional?) bugs
 */

#include "faulty_format.h"
#include "faulty_race.h"
#include "faulty_slab.h"
#include "faulty_stack.h"
#include "faulty_overflow.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <linux/fs.h>

struct dentry *dir;

const char *root = "ffaulty";

static int __init mod_init(void)
{
	pr_debug("Faulty: creating debugfs-endpoints\n");

	dir = debugfs_create_dir(root, NULL);

	if (dir == ERR_PTR(-ENODEV)) {
		pr_err
		    ("Faulty: Debugfs seems to be compiled into the kernel\n");
		return -ENODEV;
	}

	if (dir == NULL) {
		pr_err
		    ("Faulty: Cannot create debugfs-entry '%s'", root);
		return -ENOENT;
	}
	// Create endpoints for potential vulnerabilities
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
	
	if (!init_overflow(dir, "overflow"))
	    pr_debug("Faulty: Integer under/overflow at debugfs '%s/overflow'\n", root);
	else
	    pr_err("Faulty: Cannot create debugfs-entry %s/overflow\n", root);

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

module_init(mod_init);
module_exit(mod_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A Kernel Module with Faults");
MODULE_AUTHOR("Ilja Sidoroff");
