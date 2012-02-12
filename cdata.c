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
//	int i;
	int minor;
	printk(KERN_INFO "CDATA: Open\n");


	minor = MINOR(inode->i_rdev);
	if(minor >= 0){
		printk(KERN_INFO "CDATA: Minor number = %d\n", minor);
	}
	else{
		printk(KERN_INFO "CDATA: Error minor number = %d\n", minor);
		return -ENODEV;	
	}
	//MOD_INC_USE_COUNT;	//used in linux 2.4

/*
	printk(KERN_INFO "cdata_open_50000000_Lab1\n");
	for(i=0;i<50000000;i++){
		;
	}
*/
/*
	printk(KERN_INFO "cdata_open_50000000_Lab2\n");
	for(i=0;i<50000000;i++){
		schedule();
	}
*/

/*
	printk(KERN_INFO "cdata_open_5000_Lab3\n");
	for(i=0;i<5000;i++){
		current->state = TASK_UNINTERRUPTIBLE;
		schedule();
	}
*/
	return 0;
}

ssize_t cdata_read(struct file *filp, char *buf, size_t size, loff_t *off)
{
	printk(KERN_INFO "CDATA: Read\n");
	return 0;
}

ssize_t cdata_write(struct file *filp, const char *buf, size_t size, loff_t *off)
{
	printk(KERN_INFO "CDATA: Write\n");
	return 0;
}

int cdata_ioctl(struct inode *inode, struct file *filp, unsigned int ui, unsigned long ul)
{
	printk(KERN_INFO "CDATA: IOCtl\n");
	return 0;
}

int cdata_close(struct inode *inode, struct file *filp)
{
	printk(KERN_INFO "CDATA: Close\n");
	//MOD_DEC_USE_COUNT;	//used in linux 2.4
	return 0;
}

static struct file_operations cdata_fops = {
	owner:	THIS_MODULE,	//After Linux 2.6, add this and let kernel handle the count
	open:		cdata_open,
	release: 	cdata_close,
	read:		cdata_read,
	write: 	cdata_write,
	ioctl:	cdata_ioctl,
};
int cdata_init_module(void)
{
	printk(KERN_INFO "CDATA: Init module\n");
	if (register_chrdev(DEV_MAJOR, DEV_NAME, &cdata_fops) < 0) {
		printk(KERN_INFO "CDATA: Can't register driver\n");
		return -1;
	}
	return 0;
}

void cdata_cleanup_module(void)
{
	printk(KERN_INFO "CDATA: Cleanup module\n");
	unregister_chrdev(121, "cdata");
}

module_init(cdata_init_module);
module_exit(cdata_cleanup_module);

MODULE_LICENSE("GPL");
