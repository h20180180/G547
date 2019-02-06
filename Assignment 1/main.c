#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/random.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>

static dev_t first; 
static char* dev_names[] = {"_x", "_y", "_z"};   //suffixes for the names according to axis
static struct cdev c_dev;
static struct class *cls;


unsigned char x[10];   //buffer in the kernel space


//STEP 4 : Driver callback functions

static int my_open(struct inode *i, struct file *f)
{
	printk(KERN_INFO "mychar: open()\n");
	return 0;
}

static int my_close(struct inode *i, struct file *f)
{
	printk(KERN_INFO "mychar: close()\n");
	return 0;
}

static ssize_t my_read(struct file *f,char __user *buf, size_t len, loff_t *off)
{
	unsigned long ret;  //returns the number of bytes which could not be copied
	unsigned int i;
	int r;
	printk(KERN_INFO "mychar: read()\n");
	for(r=0;r<10;r++)
	{	get_random_bytes(&i, sizeof i); //generating random number
		i= i%10;
		x[r]=i;  //sending the random 10 bit number to the buffer in the kernel space consisting of 10 elements
		printk("%u",i);
	}
	ret = copy_to_user(buf,x,len);
	return 0;
}


// file operations

static struct file_operations fops =
				{
					.owner  = THIS_MODULE,
					.open   = my_open,
					.release= my_close,
					.read   = my_read,
				};

static int __init mychar_init(void)
{
	int i = 0;
	int t=0;
	printk(KERN_INFO "Char driver registered");

	//STEP 1: reserve <major, minor>
	if (alloc_chrdev_region(&first, 0, 3, "BITS-PILANI") < 0)
	{
		return -1;
	}

	//STEP 2 creation of device file
	if((cls=class_create(THIS_MODULE, "chardrv"))==NULL)
	{
		unregister_chrdev_region(first,1);
		return -1;
	}
	for (i= 0;i< 3;i++)
	{
	   if(device_create(cls, NULL,MKDEV(MAJOR(first),MINOR(first)+i), NULL, "adxl%s",dev_names[i])==NULL)
	   {
		class_destroy(cls);
		unregister_chrdev_region(first,1);
		return -1;
	   }
	}
	//STEP 3 link fops and cdev to the device node
	for(t=1;t<4;t++)
        {	cdev_init(&c_dev, &fops);
		if(cdev_add(&c_dev, first, t)==-1)
	{
		device_destroy(cls, first);
		class_destroy(cls);
		unregister_chrdev_region(first,t);
		return -1;
	}
	}
	return 0;
}
static void __exit mychar_exit(void)
{
	int i=0;
	cdev_del(&c_dev);

	for(i=0;i<3;i++)
	    device_destroy(cls, MKDEV(first,i));
	class_destroy(cls);
	unregister_chrdev_region(first, 1);
	printk(KERN_INFO "Bye : mychar driver unregistered\n\n");
}

module_init(mychar_init);
module_exit(mychar_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Deepika 2018H1400180G");
MODULE_DESCRIPTION("Assignment 1 Accelerometer");
