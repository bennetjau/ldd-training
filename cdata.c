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

#define MSG(m...) printk(KERN_INFO "CDATA: " m "\")

#define DEV_MAJOR	121
#define DEV_NAME	"debug"

struct file_operations card_fops ={
};

static int cdata_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static struct file_operations cdata_fops = {	
	open:		cdata_open,
	release:	cdata_close,
	write:	cdata_write,
};

int cdata_init_module(void)
{
	MSG("CDATA v0.1.j0");
	MSG("Copyright XXX");

	if(register_chrdev(DEV_MAJOR, DEV_NAME, &card_fops)<0){
		MSG("Couldn't register a device.");
		return -1;	
	}

	return 0;
}

void cdata_cleanup_module(void)
{
}

module_init(cdata_init_module);
module_exit(cdata_cleanup_module);

MODULE_LICENSE("GPL");
