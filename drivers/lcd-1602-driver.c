/**
 * DOC: Basic Driver for the LCD 1602
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>

#define DEVICE_NAME "lcd-1602"
#define CLASS_NAME = "lcd-1602"

dev_t dev = 0;
static struct class* device_class;

/**
 * LCD_1602_driver_init(void) - Init function for the 1602 driver
 * Return: 0 for normal execution, negative for anything abnormal
 */
static int __init LCD_1602_driver_init(void)
{
        /* Dyanmically allocate the device number */
        if (alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME) < 0) {
                pr_err("Unable to allocate device number. Exiting...\n");
                goto device_number_failed;
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

        pr_info("1602 driver inserted.\n");
        return 0;

device_failed:
        class_destroy(device_class);
class_failed:
        unregister_chrdev_region(dev, 1);
device_number_failed:
        return -1;
}

/**
 * LCD_1602_driver_exit(void) - Exit function for the 1602 driver
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
