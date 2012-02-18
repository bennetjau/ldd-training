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
	printk(KERN_INFO "CDATA: Minor number = %d\n", minor);

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

static ssize_t cdata_read(struct file *filp, char *buf, size_t size, loff_t *off)
{
	printk(KERN_INFO "CDATA: Read\n");
	return 0;
}

static ssize_t cdata_write(struct file *filp, const char *buf, size_t size, loff_t *off)
{
//	int i;
	printk(KERN_INFO "CDATA: Write\n");
/*
	//Lab1, 無排程
	printk(KERN_INFO "CDATA: None Schedule\n");
	while(1){
		printk(KERN_INFO "CDATA: in while loop\n");
	}
*/
/*

	//Lab2, 排程,stat=R, 所以一直拿到執行權利,loading會一直增加, 無法kill
	printk(KERN_INFO "CDATA: Only Schedule\n");
	while(1){
		printk(KERN_INFO "CDATA: in while loop\n");
		schedule();
	}


	//Lab3, 排程,stat=D, 無法kill,loading會一直增加
	printk(KERN_INFO "CDATA: Schedule with Task uninterruptible\n");
	while(1){
		printk(KERN_INFO "CDATA: in while loop\n");
		current->state = TASK_UNINTERRUPTIBLE;
		schedule();
	}
*/
/*
	//Lab4, 排程, stat=S, loading不會增加,但kill時,stat=R,無法kill,loading又會一直增加
	printk(KERN_INFO "CDATA: Schedule with Task interruptible\n");
	while(1){
		printk(KERN_INFO "CDATA: in while loop\n");
		current->state = TASK_INTERRUPTIBLE;
		schedule();
	}
*/
	return 0;
}

static int cdata_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	printk(KERN_INFO "CDATA: IOCtl\n");
	return 0;
}

static int cdata_close(struct inode *inode, struct file *filp)
{
	printk(KERN_INFO "CDATA: Close\n");
	//MOD_DEC_USE_COUNT;	//used in linux 2.4
	return 0;
}

/*
static int cdata_flush(struct file *filp)
{
	printk(KERN_INFO "CDATA: Flush\n");
	return 0;
}
*/

static struct file_operations cdata_fops = {
	owner:	THIS_MODULE,	//After Linux 2.6, add this and let kernel handle the count
	open:		cdata_open,
	release: 	cdata_close,
	read:		cdata_read,
	write: 	cdata_write,
	ioctl:	cdata_ioctl,
	//flush:	cdata_flush,
};

static int cdata_init_module(void)
{
	unsigned long *fb;
	int i;

	fb = ioremap(0x33f00000, 320*240*4); //screen size 320*240
	for (i=0;i<320*240;i++)
		writel(0x00ff0000, fb++); // 1 pixel ,4bite

	printk(KERN_INFO "CDATA: Init module~~~XXXX\n");
	if (register_chrdev(DEV_MAJOR, DEV_NAME, &cdata_fops) < 0) {
		printk(KERN_INFO "CDATA: Can't register driver\n");
		return -1;
	}
	return 0;
}

static void cdata_cleanup_module(void)
{
	printk(KERN_INFO "CDATA: Cleanup module\n");
	unregister_chrdev(121, "cdata");
}

module_init(cdata_init_module);
module_exit(cdata_cleanup_module);

MODULE_LICENSE("GPL");
