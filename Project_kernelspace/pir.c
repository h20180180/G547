#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/io.h>
#include <asm/uaccess.h>


#define BCM2835_PERI_BASE 0x3f000000   
#define GPIO_BASE (BCM2835_PERI_BASE + 0x200000)
#define GPIO_REGION_SIZE 0x3c
#define GPLEV_OFFSET 0x34

/* Two GPIO input pins to detect PIR output */

#define GPIO_DEFAULT 19
#define GPIO_DEFAULT2 13

#define HIGH 1
#define LOW 0
#define MODULE_NAME "pir"
#define NUM_DEVICES 2
#define BUF_SIZE 2

char val[2];

/* Device file operations*/
static ssize_t
pir_read(struct file *filep, char __user *buf, size_t len, loff_t *off);
static ssize_t
pir_write(struct file *filep, const char __user *buf, size_t len, loff_t *off);
static int pir_open(struct inode *inodep, struct file *filep);
static int pir_release(struct inode *inodep, struct file *filep);

/* Other functions*/
static void save_gpio_func_select(void);
static void restore_gpio_func_select(void);
static void pin_direction_input(void);
static void read_pin(void);

static void save_gpio_func_select2(void);
static void restore_gpio_func_select2(void);
static void pin_direction_input2(void);
static void read_pin2(void);

static dev_t devt;
static struct cdev pir_cdev;
static struct class *pir_class;
static struct device *pir_device;

/* 
  iomap will point to the first GPIO register. Since this is a void pointer,
  incrementing it will increment the address by one byte.
 */
static void __iomem *iomap;
static int func_select_reg_offset;
static int func_select_bit_offset;
static int func_select_reg_offset2;
static int func_select_bit_offset2;
static char pin_value[BUF_SIZE] = "";
static char pin_value2[BUF_SIZE] = "";
static int func_select_initial_val;
static int func_select_initial_val2;

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = pir_open,
	.release = pir_release,
	.read = pir_read,
	.write = pir_write,
};

static int gpio_num = GPIO_DEFAULT;
static int gpio_num2 = GPIO_DEFAULT2;

static int __init init_pir(void)
{
	int ret;
	
		ret = alloc_chrdev_region(&devt, 0, NUM_DEVICES, MODULE_NAME);    /* allocate memory*/
	if (ret) {
		pr_err("%s: Failed to allocate char device region.\n",
			MODULE_NAME);
		goto out;
	}

	cdev_init(&pir_cdev, &fops);               /* linking fops to cdev structure*/
	pir_cdev.owner = THIS_MODULE;
	ret = cdev_add(&pir_cdev, devt, NUM_DEVICES);
	if (ret) {
		pr_err("%s: Failed to add cdev.\n", MODULE_NAME);
		goto cdev_err;
	}

	pir_class = class_create(THIS_MODULE, "pir-test");    /* class creation*/
	if (IS_ERR(pir_class)) {
		pr_err("%s: class_create() failed.\n", MODULE_NAME);
		ret = PTR_ERR(pir_class);
		goto class_err;
	}

	pir_device = device_create(pir_class, NULL, devt, NULL, MODULE_NAME);      /*device creation*/
	if (IS_ERR(pir_device)) {
		pr_err("%s: device_create() failed.\n", MODULE_NAME);
		ret = PTR_ERR(pir_device);
		goto dev_err;
	}

	iomap = ioremap(GPIO_BASE, GPIO_REGION_SIZE);           /* maps bus memory to CPU space*/ 
	if (!iomap) {
		pr_err("%s: ioremap() failed.\n", MODULE_NAME);
		ret = -EINVAL;
		goto remap_err;
	}

	func_select_reg_offset = 4 * (gpio_num / 10);   /* selecting the func sel reg for a GPIO pin*/
	func_select_bit_offset = (gpio_num % 10) * 3;   /* selecting the offset of the starting function bit*/
	
	func_select_reg_offset2 = 4 * (gpio_num2 / 10); 
	func_select_bit_offset2 = (gpio_num2 % 10) * 3;


	save_gpio_func_select();     
	save_gpio_func_select2();
	
	pin_direction_input();  
	pin_direction_input2();

	pr_info("%s: Module loaded\n", MODULE_NAME);
	goto out;

remap_err:
	device_destroy(pir_class, devt);
dev_err:
	class_unregister(pir_class);
	class_destroy(pir_class);
class_err:
	cdev_del(&pir_cdev);
cdev_err:
	unregister_chrdev_region(devt, NUM_DEVICES);
out:
	return ret;
}

static void __exit exit_pir(void)
{
	restore_gpio_func_select();
	restore_gpio_func_select2();
	iounmap(iomap);
	device_destroy(pir_class, devt);
	class_unregister(pir_class);
	class_destroy(pir_class);
	cdev_del(&pir_cdev);
	unregister_chrdev_region(devt, NUM_DEVICES);
	pr_info("%s: Module unloaded\n", MODULE_NAME);
}

static int pir_open(struct inode *inodep, struct file *filep)
{
	return 0;
}

static int pir_release(struct inode *inodep, struct file *filep)
{
	return 0;
}

static ssize_t
pir_read(struct file *filep, char __user *buf, size_t len, loff_t *off)
{
	int err;

	read_pin();  /* Reading the state of the GPIO pins*/
	read_pin2();
	
	if ((int)pin_value[0]==49)    /* Sending 1 to user space if PIR 1 detects motion */
	{ val[0] = 1;
			printk("PIR 1 detected");
			printk("pin_value[0] = %s\n",pin_value);
		err = copy_to_user(buf, val, 1);
		printk("%d",err);
	if (err)
		return -EFAULT;
	}
	else if ((int)pin_value2[0]==49)      /* Sending 2 to user space if PIR 2 detects motion */
	{ val[0] = 2;
	printk("PIR 2 detected");
	printk("pin_value2[0] = %s\n",pin_value2);
		err = copy_to_user(buf, val, 1);
	if (err)
		return -EFAULT;
	}
	else {
		val[0]=0;
		printk("No PIR detected"); 			 /* Sending 0 to user space if no motion is detected */
		err = copy_to_user(buf,val,1);
		if (err)
		return -EFAULT;
		}

	return 0;
}
static ssize_t
pir_write(struct file *filep, const char __user *buf, size_t len, loff_t *off)
{ 
	return 0;
}



static void save_gpio_func_select(void)                 /* Initial function triplet saved*/
{
	int val;

	val = ioread32(iomap + func_select_reg_offset);
	func_select_initial_val = (val >> func_select_bit_offset) & 7;
}

static void save_gpio_func_select2(void)
{
	int val;

	val = ioread32(iomap + func_select_reg_offset2);
	func_select_initial_val2 = (val >> func_select_bit_offset2) & 7;
}

static void restore_gpio_func_select(void)          /* Initial function triplet restored*/
{
	int val;

	val = ioread32(iomap + func_select_reg_offset);
	val &= ~(7 << func_select_bit_offset);
	val |= func_select_initial_val << func_select_bit_offset;
	iowrite32(val, iomap + func_select_reg_offset);
}

static void restore_gpio_func_select2(void)
{
	int val;

	val = ioread32(iomap + func_select_reg_offset2);
	val &= ~(7 << func_select_bit_offset2);
	val |= func_select_initial_val2 << func_select_bit_offset2;
	iowrite32(val, iomap + func_select_reg_offset2);
}

static void pin_direction_input(void)			/*Setting the GPIO to input (FUNCTION BITS = 000) */
{
	int val;

	val = ioread32(iomap + func_select_reg_offset);
	val &= ~(6 << func_select_bit_offset);
	val |= 0 << func_select_bit_offset;
	iowrite32(val, iomap + func_select_reg_offset);
}

static void pin_direction_input2(void)
{
	int val;

	val = ioread32(iomap + func_select_reg_offset2);
	val &= ~(6 << func_select_bit_offset2);
	val |= 0 << func_select_bit_offset2;
	iowrite32(val, iomap + func_select_reg_offset2);
}


static void read_pin(void)				 /* GPLEV - read only register used to check value of GPIOs */ 
{
	int val;

	val = ioread32(iomap + GPLEV_OFFSET);  
	val = (val >> gpio_num) & 1;
	pin_value[0] = val ? '1' : '0';
}

static void read_pin2(void)
{
	int val;

	val = ioread32(iomap + GPLEV_OFFSET);
	val = (val >> gpio_num2) & 1;
	pin_value2[0] = val ? '1' : '0';
}

module_init(init_pir);
module_exit(exit_pir);

MODULE_AUTHOR("Deepika 2018H1400180G");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("GPIO driver to calculate speed using PIR sensors");
