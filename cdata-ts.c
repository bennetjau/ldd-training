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
#include "cdata_ioctl.h"

void cdata_bh(unsigned long);
DECLARE_TASKLET(my_tasklet, cdata_bh, NULL);

static void cdata_ts_handler(int irq, void *priv, struct pt_regs *reg)
{	//Top Half
	printk(KERN_INFO "CDATA_TS: TH....\n");
	tasklet_schedule(&my_tasklet);
}

void cdata_bh(unsigned long priv)
{
	//Bottom Half
	printk(KERN_INFO "CDATA_TS: BH....\n");
	while(1);
}

static int cdata_ts_open(struct inode *inode, struct file *filp)
{
	u32 reg;
#if 0
	reg = GPGCON;
	reg |= 0xff000000; //改24~31位元,其他就'don't care'
	GPGCON = reg;
	printk(KERN_INFO "GPGCON: %08x\n",reg);
#else
	set_gpio_ctrl(GPIO_YPON);
	set_gpio_ctrl(GPIO_YMON);
	set_gpio_ctrl(GPIO_XPON);
	set_gpio_ctrl(GPIO_XMON);
#endif

	ADCTSC = DOWN_INT | XP_PULL_UP_EN | \
		XP_AIN | XM_HIZ | YP_AIN | YM_GND | \
		XP_PST(WAIT_INT_MODE);

	/* Request touch panel IRQ */
	if (request_irq(IRQ_TC, cdata_ts_handler, 0,
		"cdata-ts", 0)){
		printk(KERN_ALERT "CDATA_TS: request irq failed. \n");
		return -1;
	}

	return 0;
}

static ssize_t cdata_ts_read(struct file *filp, char *buf, size_t size, loff_t *off)
{
	//printk(KERN_INFO "CDATA_TS: Read\n");
	return 0;
}

static ssize_t cdata_ts_write(struct file *filp, const char *buf, size_t size, loff_t *off)
{
	//printk(KERN_INFO "CDATA_TS: Write\n");
	return 0;
}

static int cdata_ts_close(struct inode *inode, struct file *filp)
{
	//printk(KERN_INFO "CDATA_TS: Close\n");
	return 0;
}

static int cdata_ts_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	//printk(KERN_INFO "CDATA_TS: Ioctl\n");
	return -ENOTTY;
}

static struct file_operations cdata_ts_fops = {
	owner:	THIS_MODULE,	//After Linux 2.6, add this and let kernel handle the count
	open:		cdata_ts_open,
	release: 	cdata_ts_close,
	read:		cdata_ts_read,
	write: 	cdata_ts_write,
	ioctl:	cdata_ts_ioctl,
};

static struct miscdevice cdata_ts_misc = {
	minor:	CDATA_TS_MINOR,
	name: 	"cdata-ts",
	fops:		&cdata_ts_fops,
};

static int cdata_ts_init_module(void)
{
	//unsigned long *fb;
	//int i;

	//加在misc的原因是會自動增加裝置,也就是不用去作mknod的動作

	if(misc_register(&cdata_ts_misc) < 0){
		printk(KERN_INFO "CDATA_TS: Can't register driver\n");
		return -1;
	}
	printk(KERN_INFO "CDATA_TS: Init module\n");

	return 0;
}

static void cdata_ts_cleanup_module(void)
{
	printk(KERN_INFO "CDATA_TS: Cleanup module\n");
	misc_deregister(&cdata_ts_misc);

}

module_init(cdata_ts_init_module);
module_exit(cdata_ts_cleanup_module);

MODULE_LICENSE("GPL");
