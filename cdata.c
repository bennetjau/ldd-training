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

#define DEV_MAJOR 121
#define DEV_NAME "cdata"
#define BUF_SIZE (128)
#define LCD_SIZE (320*240*4)

struct cdata_t{
	unsigned long *fb;
	unsigned char *buf;
	unsigned int  index;
	unsigned int offset;
	unsigned int lock;
};

static int cdata_open(struct inode *inode, struct file *filp)
{
//	int i;
	int minor;
	struct cdata_t *cdata; //使用指標是為了避免重複進入問題

	printk(KERN_INFO "CDATA: Open\n");
	

	minor = MINOR(inode->i_rdev);
	printk(KERN_INFO "CDATA: Minor number = %d\n", minor);

	cdata=kmalloc(sizeof(struct cdata_t), GFP_KERNEL);
	cdata->fb = ioremap(0x33f00000, 320*240*4);
	cdata->buf = kmalloc(BUF_SIZE, GFP_KERNEL);
	cdata->index = 0;
 	cdata->offset = 0;


	filp->private_data = (void *)cdata;



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

void flush_lcd(void *priv)
{
	struct cdata_t *cdata = (struct cdata *)priv;
	unsigned int index;
	unsigned int i;
	unsigned char *buf;
	unsigned char *fb;
	unsigned int offset;
	int j;

	buf = cdata->buf;
	fb = cdata->fb;
	index = cdata->index;
	offset = cdata->offset;

	for (i=0;i < index;i++){
		writeb(buf[i], fb+offset);
		offset++;
		if(offset > LCD_SIZE)
			offset = 0;
		 // Lab
		for (j = 0; j < 100000; j++);
	}

	cdata->index = 0;
	cdata->offset = offset;
}

void timer_task(struct timer_list *timer)
{
	current->state = TASK_RUNNING;

	timer->expires = jiffies + 2*HZ/10;
	add_timer(timer);

}

static ssize_t cdata_write(struct file *filp, const char *buf, size_t size, loff_t *off)
{

	struct cdata_t *cdata = (struct cdata*)filp->private_data;
	unsigned char *pixel;
	struct timer_list *timer;

	
	unsigned int index;
	unsigned int i;

	//printk(KERN_INFO "CDATA: Write\n");


	pixel = cdata->buf;
	index = cdata->index;

	init_timer(timer);
	timer->expires = jiffies + 2*HZ/10;
	timer->data = (struct timer_list *)timer;
	timer->function = &timer_task;

	add_timer(timer);


	for (i=0;i < size;i++){
		//printk(KERN_INFO "CDATA:Index = %d\n",cdata->index++);
		if(index >= BUF_SIZE){
			//printk(KERN_INFO "CDATA:Buffer Full\n");
			//buffer full
			flush_lcd((void *)cdata);
			index = cdata->index;  //要用狀態的思考方式,而不要用邏輯的思考方式,如index = 0;
			current->state = TASK_INTERRUPTIBLE;
			schedule();
		}
		//fb[index] = buf[i];
		copy_from_user(&pixel[index], &buf[i], 1);
		index++;
	}
	cdata->index = index;




/*
	//這樣寫不好,要考慮buffering的部份,把AP丟進來的東西先存下來,等存滿再一次送到硬體執行
	for (i = 0;i < size;i++)
		writeb(buf[i], fb);
*/

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
	struct cdata_t *cdata = (struct cdata*)filp->private_data;
	int n;
	unsigned long *fb;
	int i;

	printk(KERN_INFO "CDATA: IOCtl\n");
	switch(cmd){
		case CDATA_CLEAR:

			//n=*((int*)arg);//FIXME:dirty  ,應使用 copy_from_user()
			copy_from_user(&n, (int *)arg, 1); //???是這樣嗎？

			printk(KERN_INFO"CDATA_CLEAR: %d pixel\n",n);

			//FixME:Lock
			fb = cdata->fb;  // 在OS的撰寫,必須要先存成區域變數,再來使用,不要直接使用cdata->fb,在lock/unlock時才易操作
			//FIXME:Unlock
			for (i=0;i<n;i++)
				writel(0x00ffffff, fb++);
			break;
		case CDATA_RED:
			printk(KERN_INFO"CDATA_RED\n");

			fb = cdata->fb;
			for (i=0;i<320*240;i++)
				writel(0x00ff0000,fb++);
			break;
		/*
		case CDATA_GREEN:
			printk(KERN_INFO"CDATA_GREEN\n");

			fb = cdata->fb;
			for (i=0;i<320*240;i++)
				writel(0x0000ff00, fb++);
			break;			
		case CDATA_BLUE:
			printk(KERN_INFO"CDATA_BLUE\n");

			fb = cdata->fb;
			for (i=0;i<320*240;i++)
				writel(0x000000ff, fb++);
			break;
		case CDATA_BLACK:
			printk(KERN_INFO"CDATA_BALCK\n");

			fb = cdata->fb;
			for (i=0;i<320*240;i++)
				writel(0x00000000, fb++);
			break;
		case CDATA_WHITE:
			printk(KERN_INFO"CDATA_WHITE\n");

			fb = cdata->fb;
			for (i=0;i<320*240;i++)
			writel(0x00ffffff, fb++);
			break;
		*/
	}


	return 0;
}

static int cdata_close(struct inode *inode, struct file *filp)
{
	struct cdata_t *cdata = (struct cdata*)filp->private_data;

	printk(KERN_INFO "CDATA: Close\n");
	flush_lcd((void *)cdata);
	kfree(cdata->buf);
	kfree(cdata);
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
		writel(0x00ff0000, fb++); // 1 pixel ,4byte

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
