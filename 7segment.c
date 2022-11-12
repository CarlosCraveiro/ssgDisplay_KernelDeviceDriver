#include <linux/module.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");

static int ssgDisplay_init(void) {
	printk(KERN_ALERT "MODULE INITIALIZED!");
	return 0;
}

static void ssgDisplay_exit(void) {
	printk(KERN_ALERT "MODULE REMOVED!");
}

module_init(ssgDisplay_init);
module_exit(ssgDisplay_exit);
