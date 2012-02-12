#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <linux/input.h>
#include <asm/io.h>
#include <asm/uaccess.h>


#define DEV_MAJOR 121
#define DEV_NAME "cdata"

static int cdata_open(struct inode *inode, struct file *filp)
{
	int i;

	printk(KERN_INFO "cdata_open_50000000_SS\n");
	for(i=0;i<50000000;i++){
		schedule();
	}
/*
	printk(KERN_INFO "cdata_open_5000_T\n");
	for(i=0;i<5000;i++){
		current->state = TASK_UNINTERRUPTIBLE;
		schedule();
	}
*/
	return 0;
}

ssize_t cdata_write(struct file *filp, const char *buf, size_t size, loff_t *off)
{
	printk(KERN_INFO "cdata_write\n");
	return 0;
}

int cdata_close(struct inode *inode, struct file *filp)
{
	printk(KERN_INFO "cdata_close\n");
	return 0;
}

static struct file_operations cdata_fops = {
	open: cdata_open,
	release: cdata_close,
	write: cdata_write,
};

int cdata_init_module(void)
{
	printk(KERN_INFO "cdata_init_module\n");
	if (register_chrdev(DEV_MAJOR, DEV_NAME, &cdata_fops) < 0) {
		printk(KERN_INFO "CDATA: can't register driver\n");
		return -1;
	}
	return 0;
}

void cdata_cleanup_module(void)
{
	printk(KERN_INFO "cdata_cleanup_module\n");
	unregister_chrdev(121, "cdata");
}

module_init(cdata_init_module);
module_exit(cdata_cleanup_module);

MODULE_LICENSE("GPL");
