/**
 * DOC: Basic Driver for the LCD 1602
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>

#define DEVICE_NAME "LCD_1602"

dev_t dev = 0;

/**
 * LCD_1602_driver_init(void) - Init function for the 1602 driver
 * Return: 0 for normal execution, negative for anything abnormal
 */
static int __init LCD_1602_driver_init(void)
{
        /* Dyanmically allocate the device number */
        if(alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME) < 0) {
                pr_info("Unable to allocate device number. Exiting...\n");
                return -1;
        }

        pr_info("1602 driver inserted.\n");
        return 0;
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
