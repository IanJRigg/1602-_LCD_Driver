/**
 * DOC: Basic Driver for the LCD 1602
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/gpio.h>

#define DEVICE_NAME "lcd-1602"
#define CLASS_NAME "lcd-1602"

#define GPIO_18 18
#define GPIO_22 22
#define GPIO_23 23
#define GPIO_24 24
#define GPIO_25 25
#define GPIO_27 27

#define DISPLAY_SIZE 32

dev_t dev = 0;
static struct class* device_class;
static struct cdev lcd_1602_cdev;

static char display_buffer[DISPLAY_SIZE];

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
 * @filp: Pointer to struct file pointer abstracting the open file
 * @user_buffer: User space buffer provided for writing the read data
 * @length: Length of the provided user space buffer
 * @offset: Offset to write to in kernel space, and to read from in user space
 *
 * The contents here are nearly identical to simple_write_to_buffer() from fs.h.
 * The following will be replaced by that function if it turns out the LCD 1602
 * instructions don't require a deviation from this logic.
 *
 * Return: The number of bytes read into @user_buffer
 *
 * In addition to the retrun value, @user_buffer will contain the contents of
 * the display to the extent that they were requested. Additionally, @offset
 * will be updated to contain the new offset on the kernel space data.
 */
static ssize_t lcd_1602_read(struct file *file,
                             char __user *user_buffer,
                             size_t length,
                             loff_t *offset)
{
        loff_t pos = *offset;
        size_t result;

        /* Check against invalid offset */
        if(pos < 0) {
                return -EINVAL;
        }

        /* Check against asking for an overrun, or for asking for no data */
        if(pos >= DISPLAY_SIZE || length == 0) {
                return 0;
        }

        /* Cap the read at how much can be read from the provided position */
        if(length > DISPLAY_SIZE - pos) {
                length = DISPLAY_SIZE - pos;
        }

        /* Perform the read operation */
        result = copy_to_user(user_buffer, display_buffer + pos, length);
        if(result == length) {
                return  -EFAULT;
        }

        /* Update the provided offset with the results of the read operation */
        length -= result;
        *offset = pos + length;

        return length;
}

/**
 * lcd_1602_write() - Callback for a call to write() on the /dev/ file
 * @file: Pointer to struct file pointer abstracting the open file
 * @user_buffer: User space buffer providing the data to write
 * @length: Length of the provided user space buffer
 * @offset: Offset to write to in user space, and to read from in kernel space
 *
 * The contents here are nearly identical to simple_write_to_buffer() from fs.h.
 * The following will be replaced by that function if it turns out the LCD 1602
 * instructions don't require a deviation from this logic.
 *
 * Return: The number of bytes read from @user_buffer
 *
 * In addition to the return value, @offset will be updated to contain the new
 * offset on the provided data.
 */
static ssize_t lcd_1602_write(struct file *file,
                              const char __user *user_buffer,
                              size_t user_length,
                              loff_t *offset)
{
        loff_t pos = *offset;
        size_t result;
        size_t write_length = DISPLAY_SIZE;

        /* Check against invalid offset */
        if(pos < 0) {
                return -EINVAL;
        }

        /* Check against asking for an overrun, or for asking for no data */
        if((pos >= user_length) || (user_length == 0)) {
                /* I really don't like this. The internet is not totally
                 * clear on whether the return value should be 0, or full
                 * length in this case. I'll leave this as a todo for future
                 * investigation, but as it stands, returning 0 causes an
                 * infinite loop when writing >32 characters /dev/lcd-1602
                 */
                return user_length;
        }

        /* Cap the write at how much can be written to the provided position */
        if(write_length > (user_length - pos)) {
                write_length = user_length - pos;
        }

        /* Perform the write operation */
        result = copy_from_user(display_buffer + pos, user_buffer, user_length);
        if(result == write_length) {
                return  -EFAULT;
        }

        /* Update the provided offset with the results of the read operation */
        write_length -= result;
        *offset = pos + write_length;

        return write_length;
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

        /* TODO: Look into ways to make this into a single function */
        /* Reserve GPIO_18 */
        if(gpio_is_valid(GPIO_18) == false) {
                pr_err("GPIO_%d is invalid\n", GPIO_18);
                goto device_failed;
        } else if(gpio_request(GPIO_18, "GPIO_18") < 0) {
                pr_err("Failed to acquire GPIO_%d\n", GPIO_18);
                goto gpio_18_failed;
        }

        /* Reserve GPIO_22 */
        if(gpio_is_valid(GPIO_22) == false) {
                pr_err("GPIO_%d is invalid\n", GPIO_22);
                goto device_failed;
        } else if(gpio_request(GPIO_22, "GPIO_22") < 0) {
                pr_err("Failed to acquire GPIO_%d\n", GPIO_22);
                goto gpio_22_failed;
        }

        /* Reserve GPIO_23 */
        if(gpio_is_valid(GPIO_23) == false) {
                pr_err("GPIO_%d is invalid\n", GPIO_23);
                goto device_failed;
        } else if(gpio_request(GPIO_23, "GPIO_23") < 0) {
                pr_err("Failed to acquire GPIO_%d\n", GPIO_23);
                goto gpio_23_failed;
        }

        /* Reserve GPIO_24 */
        if(gpio_is_valid(GPIO_24) == false) {
                pr_err("GPIO_%d is invalid\n", GPIO_24);
                goto device_failed;
        } else if(gpio_request(GPIO_24, "GPIO_24") < 0) {
                pr_err("Failed to acquire GPIO_%d\n", GPIO_24);
                goto gpio_24_failed;
        }

        /* Reserve GPIO_25 */
        if(gpio_is_valid(GPIO_25) == false) {
                pr_err("GPIO_%d is invalid\n", GPIO_25);
                goto device_failed;
        } else if(gpio_request(GPIO_25, "GPIO_25") < 0) {
                pr_err("Failed to acquire GPIO_%d\n", GPIO_25);
                goto gpio_25_failed;
        }

        /* Reserve GPIO_27 */
        if(gpio_is_valid(GPIO_27) == false) {
                pr_err("GPIO_%d is invalid\n", GPIO_27);
                goto device_failed;
        } else if(gpio_request(GPIO_27, "GPIO_27") < 0) {
                pr_err("Failed to acquire GPIO_%d\n", GPIO_27);
                goto gpio_27_failed;
        }

        pr_info("LCD 1602 driver inserted.\n");
        return 0;

gpio_27_failed:
        gpio_free(GPIO_27);
gpio_25_failed:
        gpio_free(GPIO_25);
gpio_24_failed:
        gpio_free(GPIO_24);
gpio_23_failed:
        gpio_free(GPIO_23);
gpio_22_failed:
        gpio_free(GPIO_22);
gpio_18_failed:
        gpio_free(GPIO_18);
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
        gpio_free(GPIO_27);
        gpio_free(GPIO_25);
        gpio_free(GPIO_24);
        gpio_free(GPIO_23);
        gpio_free(GPIO_22);
        gpio_free(GPIO_18);
        class_destroy(device_class);
        unregister_chrdev_region(dev, 1);
        pr_info("1602 driver removed.\n");
}
 
module_init(LCD_1602_driver_init);
module_exit(LCD_1602_driver_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ian Rigg");
MODULE_DESCRIPTION("A driver for the LCD 1602");
MODULE_VERSION("0:0.1");
