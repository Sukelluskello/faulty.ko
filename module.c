/*
 * A kernel module with intentional (and unintentional?) bugs
 */

#include <linux/module.h>
#include <linux/kernel.h>
//#include <linux/init.h>
//#include <linux/slab.h>
//#include <linux/list.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A Kernel Module with faults");
MODULE_AUTHOR("Ilja Sidoroff");


static int mod_init(void)
{

}

static void mod_cleanup(void)
{

}

module_init(mod_init);
module_exit(mod_cleanup);
