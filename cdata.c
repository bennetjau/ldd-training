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
#include <linux/semaphore.h>
#include <linux/spinlock.h>
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
	
	struct timer_list flush_timer;
	struct timer_list sched_timer;
	

	//DECLARE_WAIT_QUEUE_HEAD(wq);
	wait_queue_head_t	wq;
	struct semaphore sem;
	spinlock_t		lock;
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


	init_timer(&cdata->flush_timer);
	init_timer(&cdata->sched_timer);

	init_waitqueue_head(&cdata->wq);

	sema_init(&cdata->sem, 1);
	spin_lock_init(&cdata->lock);

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

void flush_lcd(unsigned long priv)
{
	struct cdata_t *cdata = (struct cdata *)priv;
	unsigned int index;
	unsigned int i;
	unsigned char *buf;
	unsigned char *fb;
	unsigned int offset;
	int j;

	//在中斷模式下跑,所以不會被中斷,不用使用spin_lock_irqsave
	//spin_lock(&cdata->lock);
	//在SMP下,雖然已經處在local的中斷模式下,但是另一個處理器還是有可能被中斷,所以仍需要用irqsave來處理	
	spin_lock_irqsave(&cdata->lock);	
	buf = cdata->buf;
	fb = (unsigned char *)cdata->fb;
	index = cdata->index;
	offset = cdata->offset;
	spin_unlock_irqrestore(&cdata->lock);
	//spin_unlock(&cdata->lock);

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

void cdata_wake_up(unsigned long priv)
{
	struct cdata_t *cdata = (struct cdata *)priv;
	//3. FIXME:Wake up process
	struct timer_list *sched;
	wait_queue_head_t *wq;

	sched = &cdata->sched_timer;
	wq = &cdata->wq;

	wake_up(wq);

	sched->expires = jiffies + 10;
	add_timer(sched);

}

static ssize_t cdata_write(struct file *filp, const char *buf, size_t size, loff_t *off)
{

	struct cdata_t *cdata = (struct cdata*)filp->private_data;
	unsigned char *pixel;
	struct timer_list *timer;
	struct timer_list *sched;
	
	unsigned int index;
	unsigned int i;

	wait_queue_t wait;
	wait_queue_head_t	*wq;
	//printk(KERN_INFO "CDATA: Write\n");

	//要加lock/unlock時,應該放在如下:
	//盡可能要lock的東西集中,不要到處加
	//LOCK時機;call down();
	//處理reentrency的問題:down,up
	//down();

	down_interruptible(&cdata->sem);

	//處理Process Context code, Interrupt Context code 之間的共用資料：spin
	//spin_lock_irqsave();


	//不是在中斷模式下跑,所以會被中斷,要使用spin_lock_irqsave, spin_unlock_irqrestore
	spin_lock_irqsave(&cdata->lock);
	pixel = cdata->buf;
	index = cdata->index;
	//
	//spin_unlock_irqrestore();
	spin_unlock_irqrestore(&cdata->lock);

	timer = &cdata->flush_timer;
	sched = &cdata->sched_timer;
	wq = &cdata->wq;
	//UNLOCK時機;call up();
	//up();
	up(&cdata->sem);


	for (i=0;i < size;i++){
		//printk(KERN_INFO "CDATA:Index = %d\n",cdata->index++);
		if(index >= BUF_SIZE){
			//printk(KERN_INFO "CDATA:Buffer Full\n");
			//buffer full

			//kernel 目前的時間就叫jiffies.電腦開機時,會把jiffies設成0, 每隔一秒,jiffies會累加1HZ的值, HZ=100,表kernel timer的頻率

			//1. FIXME:Kernel scheduling
			down_interruptible(&cdata->sem);
			cdata->index = index; //why add this????
			up(&cdata->sem);
			timer->expires = jiffies + 5*HZ;
			timer->function = flush_lcd;
			timer->data = (unsigned long) cdata;
			add_timer(timer);


			//2. FIXME: Process scheduling

			sched->expires = jiffies + 10;
			sched->function = cdata_wake_up;
			sched->data = (unsigned long) cdata;
			add_timer(sched);

			wait.flags = 0;
			wait.task = current;
			add_wait_queue(wq, &wait);
repeat:
			current->state = TASK_INTERRUPTIBLE;
			schedule();
			down_interruptible(&cdata->sem);
			index = cdata->index;  //要用狀態的思考方式,而不要用邏輯的思考方式,如index = 0;
			up(&cdata->sem);
			if(index != 0)
				goto repeat;

			remove_wait_queue(wq, &wait);
			del_timer(sched);			
		}
		//fb[index] = buf[i];
		copy_from_user(&pixel[index], &buf[i], 1);
		index++;
	}
	down_interruptible(&cdata->sem);
	cdata->index = index;
	up(&cdata->sem);



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

			//copy_from_user()屬於blocking API
			//n=*((int*)arg);//FIXME:dirty  ,應使用 copy_from_user()
			copy_from_user(&n, (int *)arg, 1); //???是這樣嗎？    
			//copy_from_user(&n, &arg, 1); //???還是這樣嗎？
			//get_user(n,arg); <-- 簡單的type可以用這個

			printk(KERN_INFO"CDATA_CLEAR: %d pixel\n",n);

			//FixME:Lock
			fb = cdata->fb;  // 在OS的撰寫,必須要先存成區域變數,再來使用,不要直接使用cdata->fb,在lock/unlock時才易操作
			//FIXME:Unlock
			for (i=0;i<n;i++)
				writel(0x00ffffff, fb++);

			return 0;
			break;
		case CDATA_RED:
			printk(KERN_INFO"CDATA_RED\n");

			fb = cdata->fb;
			for (i=0;i<320*240;i++)
				writel(0x00ff0000,fb++);

			return 0;
			break;
		/*
		case CDATA_GREEN:
			printk(KERN_INFO"CDATA_GREEN\n");

			fb = cdata->fb;
			for (i=0;i<320*240;i++)
				writel(0x0000ff00, fb++);
			return 0;
			break;			
		case CDATA_BLUE:
			printk(KERN_INFO"CDATA_BLUE\n");

			fb = cdata->fb;
			for (i=0;i<320*240;i++)
				writel(0x000000ff, fb++);
			return 0;
			break;
		case CDATA_BLACK:
			printk(KERN_INFO"CDATA_BALCK\n");

			fb = cdata->fb;
			for (i=0;i<320*240;i++)
				writel(0x00000000, fb++);
			return 0;
			break;
		case CDATA_WHITE:
			printk(KERN_INFO"CDATA_WHITE\n");

			fb = cdata->fb;
			for (i=0;i<320*240;i++)
			writel(0x00ffffff, fb++);
			return 0;
			break;
		*/
	}

	return -ENOTTY;
}

static int cdata_close(struct inode *inode, struct file *filp)
{
	struct cdata_t *cdata = (struct cdata*)filp->private_data;

	printk(KERN_INFO "CDATA: Close\n");
	flush_lcd((unsigned long)cdata);
	del_timer(&cdata->flush_timer);
	kfree(cdata->buf);
	kfree(cdata);
	//MOD_DEC_USE_COUNT;	//used in linux 2.4
	return 0;
}


static int cdata_flush(struct file *filp)
{
	printk(KERN_INFO "CDATA: Flush\n");
	return 0;
}

int cdata_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long from;
	unsigned long to;
	unsigned long size;
	printk(KERN_INFO "CDATA: cdata_mmap\n");

	from= vma->vm_start;
	to = 0x33f00000;
	size = vma->vm_end-vma->vm_start;
	
	//一次一大塊的作法,只能在確定實體記憶體是連續的,才能如此使用
	//remap_page_range(from, to, size, PAGE_SHARED);
	

	//如果無法確定,一般的作法一定就要一個page一個page來作
	while(size){
		remap_page_range(from, to, PAGE_SIZE, PAGE_SHARED);
		from += PAGE_SIZE;
		to += PAGE_SIZE;
		size -= PAGE_SIZE;
	}

	printk(KERN_INFO "CDATA: start = %8x\n",vma->vm_start);
	printk(KERN_INFO "CDATA: end = %8x\n",vma->vm_end);
	return 0;
}

static struct file_operations cdata_fops = {
	owner:	THIS_MODULE,	//After Linux 2.6, add this and let kernel handle the count
	open:		cdata_open,
	release: 	cdata_close,
	read:		cdata_read,
	write: 	cdata_write,
	ioctl:	cdata_ioctl,
	flush:	cdata_flush,
	mmap:		cdata_mmap,
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
