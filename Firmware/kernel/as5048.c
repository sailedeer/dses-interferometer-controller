#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/ioctl.h>
#include <linux/spi/spi.h>
#include <linux/uaccess.h>
#include <linux/types.h>

#include "as5048.h"

#define MAN_DEV_NAME			"asm,as5048"
#define DEV_NAME				"as5048"

/*
*   This major number falls into the local/experimental range
*   as specified by the LANANA Device List here:
*   http://mirrors.mit.edu/kernel/linux/docs/lanana/device-list/devices-2.6.txt
*/
#define AS_MAJOR				240
#define AS_FIRST_MINOR			0
#define N_MINORS				1

#define BUFSIZE					8

#define DIR_BIT					BIT(7)

struct as_data {
	// character device instance for this device
	struct cdev 		char_dev;

	// host side spi device
	struct spi_device	*spi_dev;

	// tx/rx buffers
	unsigned char		*tx;
	unsigned char		*rx;

	// TODO: do we care about any other data?
};

// class for our character device
static struct class *as_class = NULL;

static int as5048_open(struct inode *inode, struct file *filp) {
	struct as_data *as = NULL;

	as = container_of(inode->i_cdev, struct as_data, char_dev);
	if (!as) {
		return -ENODEV;
	}

	if (as->tx && as->rx) {
		// already open
		return -EBUSY;
	}

	as->tx = kmalloc(BUFSIZE, GFP_KERNEL);
	if (!as->tx) {
		return -ENOMEM;
	}

	as->rx = kmalloc(BUFSIZE, GFP_KERNEL);
	if (!as->rx) {
		kfree(as->tx);
		return -ENOMEM;
	}
	filp->private_data = as;

	// no seeking, behave like a stream
	stream_open(inode, filp);
	return 0;
}

// write to arbitrary address
static ssize_t as5048_write(struct file *filp, const char __user *buf,
								size_t count, loff_t *poff) {
	struct as_data *as;
	ssize_t status;
	unsigned long missing;

	if (count > BUFSIZE) {
		return -EMSGSIZE;
	}

	as = filp->private_data;
	missing = copy_from_user(as->tx, buf, count);
	if (missing == 0) {
		as->tx[0] &= ~DIR_BIT;
		status = spi_write(as->spi_dev, as->tx, count);
	} else {
		status = -EFAULT;
	}
	return status;
}

// read out position data
static ssize_t as5048_read(struct file *filp, char __user *buf,
								size_t count, loff_t *poff) {
	struct as_data *as;
	struct position_data data;
    unsigned short data_temp;
	ssize_t status;
	unsigned long missing;
    static const unsigned short ANGLE_ADDR = 0x3FFF;
    static const unsigned short MAG_ADDR = 0x3FFE;

	if (count != sizeof(struct position_data)) {
		return -EMSGSIZE;
	}

	as = filp->private_data;
    as->tx[0] = ((MAG_ADDR & 0xFF) >> 8) | (1 << 6);
    as->tx[1] = MAG_ADDR & 0xFF;

    // use write/read wrapper since CS needs to be de-selected every 16 bytes on this chip
    status = spi_write_then_read(as->spi_dev, as->tx, 2, as->rx, 2);
    if (status < 0) {
        return status;
    }
    memcpy(&data_temp, as->rx, 2);

    as->tx[0] = ((ANGLE_ADDR & 0xFF) >> 8) | (1 << 6);
    as->tx[1] = ANGLE_ADDR & 0xFF;
    status = spi_write_then_read(as->spi_dev, as->tx, 2, as->rx, 2);
    if (status < 0) {
        return status;
    }
    memcpy(&data.mag, as->rx, 2);
    memcpy(&data.angle, &data_temp, 2);

    missing = copy_to_user(buf, &data, sizeof(struct position_data));
    if (missing == 0) {
        return 0;
    } else {
        return -EIO;
    }
}

// fine grained commands for manipulating device config
static ssize_t as5048_ioctl(struct file * filp, unsigned int cmd, unsigned long arg) {
    struct as_data *as;
    if (_IOC_TYPE(cmd) != AS_MAGIC) {
        return -ENOTTY;
    }

    as = filp->private_data;

    switch (cmd) {
        case AS_R_REG:
            break;
        case AS_W_REG:
            break;
    }

	return 0;
}

static int as5048_release(struct inode *inode, struct file *filp) {
	struct as_data *as;
	as = filp->private_data;
	filp->private_data = NULL;

	kfree(as->tx);
	as->tx = NULL;

	kfree(as->rx);
	as->rx = NULL;
	kfree(as);
	return 0;
}

static const struct file_operations as_fops = {
	.owner =	THIS_MODULE,
	.write =	as5048_write,
	.read =		as5048_read,
	.unlocked_ioctl = as5048_ioctl,
	.open =		as5048_open,
	.release =	as5048_release,
	.llseek =	no_llseek,
};

static const struct spi_device_id as5048_spi_ids[] = {
	{ .name = DEV_NAME },
	{},
};
MODULE_DEVICE_TABLE(spi, as5048_spi_ids);

#ifdef CONFIG_OF
static const struct of_device_id as5048_dt_ids[] = {
	{ .compatible = MAN_DEV_NAME },
	{},
};
MODULE_DEVICE_TABLE(of, as5048_dt_ids);
#endif

static int as5048_probe(struct spi_device *spi_dev) {
	int				status;
	dev_t			devt;
	struct device 	*dev;
	struct as_data *as;
	struct spi_transfer msg;

	// allocate data for our driver
	as = kzalloc(sizeof(struct as_data), GFP_KERNEL);
	if (!as) {
		return -ENOMEM;
	}

	devt = MKDEV(AS_MAJOR, AS_FIRST_MINOR);
	dev = device_create(as_class, &spi_dev->dev, devt, as, DEV_NAME);
	if (IS_ERR(dev)) {
		kfree(as);
		return PTR_ERR(dev);
	}

	// initialize our device with the given file operations
	cdev_init(&as->char_dev, &as_fops);
	as->char_dev.owner = THIS_MODULE;
	status = cdev_add(&as->char_dev, devt, N_MINORS);
	if (status < 0) {
		device_destroy(as_class, devt);
		kfree(as);
		return status;
	}

	as->spi_dev = spi_dev;
	spi_set_drvdata(spi_dev, as);
	return 0;
}

static int as5048_remove(struct spi_device *spi_dev)
{
	struct as_data	*as = spi_get_drvdata(spi_dev);
	device_destroy(as_class, MKDEV(AS_MAJOR, AS_FIRST_MINOR));
	kfree(as);
	return 0;
}

static struct spi_driver as_spi_driver = {
	.driver = {
		.name =			DEV_NAME,
		.owner = 		THIS_MODULE,
	},
	.probe =		as5048_probe,
	.remove =		as5048_remove,
	.id_table =		as5048_spi_ids,
};

static int __init as5048_init(void) {
	dev_t devt;
    int status;

	pr_info("Initializing as5048 Device Driver module...");
	devt = MKDEV(AS_MAJOR, AS_FIRST_MINOR);
	status = register_chrdev_region(devt, N_MINORS, DEV_NAME);
	if (status < 0) {
		pr_err("Failed to allocate character device numbers.");
		return status;
	}

	as_class = class_create(THIS_MODULE, DEV_NAME);
	if (IS_ERR(as_class)) {
		unregister_chrdev_region(devt, N_MINORS);
		return PTR_ERR(as_class);
	}

	status = spi_register_driver(&as_spi_driver);
	if (status < 0) {
		unregister_chrdev_region(devt, N_MINORS);
		class_destroy(as_class);
		pr_err("Failed to register driver.");
		return status;
	}

    pr_info("as5048 Device Driver initialized.");
    return 0;
}

static void __exit as5048_exit(void) {
	dev_t devt;
    pr_info("Removing as5048 Device Driver module...");
	spi_unregister_driver(&as_spi_driver);
	class_destroy(as_class);

	devt = MKDEV(AS_MAJOR, AS_FIRST_MINOR);
	unregister_chrdev_region(devt, N_MINORS);
    pr_info("as5048 Device Driver removed.");
}

module_init(as5048_init);
module_exit(as5048_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("as5048 Device Driver");
MODULE_AUTHOR("sailedeer, sailedeer11@gmail.com");


