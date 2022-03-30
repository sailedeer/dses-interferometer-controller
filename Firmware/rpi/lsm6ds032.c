#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/gpio.h>
#include <linux/spi.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>

MODULE_AUTHOR("sailedeer");
MODULE_LICENSE("GPL");


