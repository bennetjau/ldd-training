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

/*FIXME 改成private data */
//int x;
//int y;

//struct input_dev ts_input;

//priviate data
struct cdata_ts{
	struct input_dev ts_input;
	int x;
	int y;
};

static int ts_input_open(struct input_dev *dev)
{
}

static int ts_input_close(struct input_dev *dev)
{
}

static void cdata_ts_handler(int irq, void *priv, struct pt_regs *reg)
{	//Top Half

	struct cdata_ts *cdata = (struct cdata_ts *) priv;

	printk(KERN_INFO "CDATA_TS: TH....\n");

	/* FIXME:read(x,y) from ADC */
	cdata->x = 100;
	cdata->y = 100;


	//read hardware info需在TF,因為需要發生中斷的當下的訊息存下來,若放在BH,則當BH執行時,可能就lost了

	my_tasklet.data = (unsigned long)cdata;
	tasklet_schedule(&my_tasklet);
}

void cdata_bh(unsigned long priv)
{
	//Bottom Half
	struct cdata_ts *cdata = (struct cdata_ts *) priv;
	struct input_dev *dev = &cdata->ts_input;
	printk(KERN_INFO "CDATA_TS: BH....\n");
	input_report_abs(dev, ABS_X, cdata->x);
	input_report_abs(dev, ABS_Y, cdata->y);	

}

static int cdata_ts_open(struct inode *inode, struct file *filp)
{
	struct cdata_ts * cdata;
	cdata = kmalloc(sizeof(struct cdata_ts), GFP_KERNEL);

#if 0
	u32 reg;

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
		"cdata-ts", (void *) cdata)){
		printk(KERN_ALERT "CDATA_TS: request irq failed. \n");
		return -1;
	}

	//等裝置建立起來後,才去register input subsystem
	/** handling input device ***/
	cdata->ts_input.name = "cdata-ts";
	cdata->ts_input.open = ts_input_open;
	cdata->ts_input.close = ts_input_close;
	//capabilties
	//可回報絕對x,y
	cdata->ts_input.absbit[0] = BIT(ABS_X) | BIT(ABS_Y);
	//可回報enter
	//ts_input.evbit[0] = BIT(KEY_ENTER) | BIT(KEY_F5);

	input_register_device(&cdata->ts_input);
	cdata->x = 0;
	cdata->y = 0;
	filp->private_data = (void *)cdata;


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



	//需改成input device,原因是需要把x,y值由kernel送到user space,此時用input subsystem比較適合
	//但實際應用通常會用到多個subsystem,需要混和subystem的作法,所以仍然是先註冊misc
	/*
	if(input_register_device(&cdata_ts_input) < 0){
		printk(KERN_INFO "CDATA_TS: Can't register driver\n");
		return -1;	
	}
	*/
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
