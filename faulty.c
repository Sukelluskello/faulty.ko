/*
 * A kernel module with intentional (and unintentional?) bugs
 */

#include <linux/module.h>
#include <linux/kernel.h>

// TODO split into files
int init_stack_buffer_overflow(void);
void good_function(void);
void bad_function(void);

static int __init mod_init(void)
{
    pr_debug("** Loaded faulty kernel module\n");
    if (!init_stack_buffer_overflow())
	pr_debug("*** Stack buffer overflow at xxx\n");
    return 0;
}

static void __exit mod_exit(void)
{
    pr_debug("** Unloaded faulty kernel module\n");
}

int init_stack_buffer_overflow()
{
    return 0;
}

void good_function() {
    pr_debug("** <3 This is a good function and should be called <3\n");
}

void bad_function() {
    pr_debug(" :( :( :( This function should not be called :( :( :( \n");
}

module_init(mod_init);
module_exit(mod_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A Kernel Module with Faults");
MODULE_AUTHOR("Ilja Sidoroff");
