/**
 * DOC: Basic Driver for the LCD 1602
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>

#define DEVICE_NAME "lcd-1602"
#define CLASS_NAME "lcd-1602"

dev_t dev = 0;
static struct class* device_class;
static struct cdev lcd_1602_cdev;

/**
 * lcd_1602_open() - Callback for a call to open() on the /dev/ file
 * @inode:
 * @file:
 *
 * Return:
 */
static int lcd_1602_open(struct inode *inode, struct file *file)
{
        pr_info("LCD_1602 device file opened!\n");
        return 0;
}

/**
 * lcd_1602_release() - Callback for a call to close() on the /dev/ file
 * @inode:
 * @file:
 *
 * Return:
 */
static int lcd_1602_release(struct inode *inode, struct file *file)
{
        pr_info("LCD_1602 device file closed!\n");
        return 0;
}

/**
 * lcd_1602_read() - Callback for a call to read() on the /dev/ file
 * @file:
 * @buffer:
 * @length: Length of the provided buffer
 * @offset:
 *
 * Return:
 */
static ssize_t lcd_1602_read(struct file *file,
                             char __user *buffer,
                             size_t length,
                             loff_t *offset)
{
        pr_info("LCD_1602 device file read from!\n");
        return 0;
}

/**
 * lcd_1602_write() - Callback for a call to write() on the /dev/ file
 * @file:
 * @buffer:
 * @length:
 * @offset:
 *
 * Return: The length of data written
 */
static ssize_t lcd_1602_write(struct file *file,
                              const char __user *buffer,
                              size_t length,
                              loff_t *offset)
{
        pr_info("LCD_1602 device file written to!\n");
        return len;
}

static const struct file_operations fops =
{
        .owner = THIS_MODULE,
        .open = lcd_1602_open,
        .release = lcd_1602_release,
        .read = lcd_1602_read,
        .write = lcd_1602_write
};

/**
 * LCD_1602_driver_init() - Init function for the 1602 driver
 *
 * Return: 0 for normal execution, negative for anything abnormal
 */
static int __init LCD_1602_driver_init(void)
{
        /* Dyanmically allocate the device number */
        if (alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME) < 0) {
                pr_err("Unable to allocate device number. Exiting...\n");
                goto device_number_failed;
        }

        /* Create the character device, and register it with the system */
        cdev_init(&lcd_1602_cdev, &fops);
        if(cdev_add(&lcd_1602_cdev, dev, 1) < 0) {
                pr_err("Unable to add device to the system\n");
                goto cdev_failed;
        }

        /* Create the entry under /sys/class/ */
        device_class = class_create(THIS_MODULE, CLASS_NAME);
        if (device_class == NULL) {
                pr_err("Unable to create device class. Exiting...\n");
                goto class_failed;
        }

        /* Create the entry under /dev/ */
        if (device_create(device_class, NULL, dev, NULL, DEVICE_NAME) == NULL) {
                pr_err("Unable to create the device. Exiting...\n");
                goto device_failed;
        }

        pr_info("LCD 1602 driver inserted.\n");
        return 0;

device_failed:
        class_destroy(device_class);
cdev_failed:
class_failed:
        unregister_chrdev_region(dev, 1);
device_number_failed:
        return -1;
}

/**
 * LCD_1602_driver_exit() - Exit function for the 1602 driver
 *
 * Return: Nothing
 */
static void __exit LCD_1602_driver_exit(void)
{
        unregister_chrdev_region(dev, 1);
        pr_info("1602 driver removed.\n");
}
 
module_init(LCD_1602_driver_init);
module_exit(LCD_1602_driver_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ian Rigg");
MODULE_DESCRIPTION("A driver for the LCD 1602");
MODULE_VERSION("0:0.1");
