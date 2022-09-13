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

#include "lsm6ds032.h"

struct lsm_data {
	// character device instance for this device
	struct cdev 		char_dev;

	// host side spi device
	struct spi_device	*spi_dev;

	// tx/rx buffers
	unsigned char		*tx;
	unsigned char		*rx;

	// cached device config info
	lsm6ds032_accel_scale_t accel_scale;
	lsm6ds032_gyro_scale_t gyro_scale;
	lsm6ds032_odr_t accel_odr;
	lsm6ds032_odr_t gyro_odr;
	lsm6ds032_gyro_lpf_bw_t gyro_lpf_bw;
};

// class for our character device
static struct class *lsm_class = NULL;

static inline int lsm6ds032_configure(struct lsm_data *lsm) {
	int status;

	// TODO: masks for register values
	lsm->tx[0] = LSM6DS032_CTRL1_XL_ADDR;
	lsm->tx[1] = 0b01000000;

	status = spi_write(lsm->spi_dev, lsm->tx, 2);
	if (status < 0) {
		return status;
	}

	lsm->tx[0] = LSM6DS032_CTRL2_G_ADDR;
	lsm->tx[1] = 0b01000010;
	status = spi_write(lsm->spi_dev, lsm->tx, 2);
	if (status < 0) {
		return status;
	}

	lsm->tx[0] = LSM6DS032_CTRL4_C_ADDR;
	lsm->tx[1] = 0b00101010;
	status = spi_write(lsm->spi_dev, lsm->tx, 2);
	if (status < 0) {
		return status;
	}

	lsm->tx[0] = LSM6DS032_CTRL6_C_ADDR;
	lsm->tx[1] = 0b00000111;
	status = spi_write(lsm->spi_dev, lsm->tx, 2);
	return status;
}

static int lsm6ds032_open(struct inode *inode, struct file *filp) {
	struct lsm_data *lsm = NULL;
	int status;

	lsm = container_of(inode->i_cdev, struct lsm_data, char_dev);
	if (!lsm) {
		return -ENODEV;
	}

	if (lsm->tx && lsm->rx) {
		// already open
		return -EBUSY;
	}

	lsm->tx = kmalloc(BUFSIZE, GFP_KERNEL);
	if (!lsm->tx) {
		return -ENOMEM;
	}

	lsm->rx = kmalloc(BUFSIZE, GFP_KERNEL);
	if (!lsm->rx) {
		kfree(lsm->tx);
		return -ENOMEM;
	}
	filp->private_data = lsm;

	// configure the device for normal operation
	status = lsm6ds032_configure(lsm);
	if (status < 0) {
		kfree(lsm->tx);
		kfree(lsm->rx);
		pr_err("Failed to configure LSM6DS032 for normal operation.");
		return status;
	}

	// no seeking, behave like a stream
	stream_open(inode, filp);
	return status;
}

// write to arbitrary address
static ssize_t lsm6ds032_write(struct file *filp, const char __user *buf,
								size_t count, loff_t *poff) {
	struct lsm_data *lsm;
	ssize_t status = 0;
	unsigned long missing;

	if (count > BUFSIZE) {
		return -EMSGSIZE;
	}

	lsm = filp->private_data;
	missing = copy_from_user(lsm->tx, buf, count);
	if (missing == 0) {
		lsm->tx[0] &= ~DIR_BIT;
		status = spi_write(lsm->spi_dev, lsm->tx, count);
	} else {
		status = -EFAULT;
	}
	return status;
}

// read out gyro/accel data
static ssize_t lsm6ds032_read(struct file *filp, char __user *buf,
								size_t count, loff_t *poff) {
	struct lsm_data *lsm;
	struct motion_data data;
	ssize_t status;
	unsigned char ctrl_3_c_old;
	unsigned long missing;

	if (count != sizeof(struct motion_data)) {
		return -EMSGSIZE;
	}

	lsm = filp->private_data;

	// read contents of CTRL3_C
	lsm->tx[0] = DIR_BIT | LSM6DS032_CTRL3_C_ADDR;

	status = spi_write_then_read(lsm->spi_dev, lsm->tx, 1, lsm->rx, 1);
	if (status < 0) {
		return status;
	}

	// ensure address gets incremented on our next transfer
	ctrl_3_c_old = lsm->rx[0];
	lsm->tx[0] &= ~DIR_BIT;
	lsm->tx[1] = ctrl_3_c_old | BIT(2);

	status = spi_write(lsm->spi_dev, lsm->tx, 2);
	if (status < 0) {
		return status;
	}

	lsm->tx[0] = LSM6DS032_OUTX_L_G_ADDR | DIR_BIT;

	status = spi_write_then_read(lsm->spi_dev, lsm->tx, 1, lsm->rx,
									12);
	if (status < 0) {
		return status;
	}

	data.gyro.x = lsm->rx[0] | (lsm->rx[1] << 8);
	data.gyro.y = lsm->rx[2] | (lsm->rx[3] << 8);
	data.gyro.z = lsm->rx[4] | (lsm->rx[5] << 8);

	data.accel.x = lsm->rx[6]| (lsm->rx[7] << 8);
	data.accel.y = lsm->rx[8]| (lsm->rx[9] << 8);
	data.accel.z =lsm->rx[10] | (lsm->rx[11] << 8);

	missing = copy_to_user(buf, &data, sizeof(struct motion_data));
	if (missing == 0) {
		return 0;
	} else {
		return -EIO;
	}
}

// fine grained commands for manipulating device config
static long lsm6ds032_ioctl(struct file * filp, unsigned int cmd, unsigned long arg) {
	int status;
    ssize_t missing;
	struct lsm_data *lsm;
	struct lsm6ds032_reg_io io;

	if (_IOC_TYPE(cmd) != LSM_MAGIC) {
		return -ENODEV;
	}

	lsm = filp->private_data;
	missing = copy_from_user(&io, (struct lsm6ds032_reg_io __user *) arg, sizeof(struct lsm6ds032_reg_io));

    if (missing > 0) {
        return -EFAULT;
    }

	switch (cmd) {
		case LSM_R_REG:
            lsm->tx[0] = io.addr;
			break;
        case LSM_W_REG:
            lsm->tx[0] = io.addr;
            lsm->tx[1] = io.value;
            status = spi_write(lsm->spi_dev, lsm->tx, 2);
            break;
        default:
            status = -EFAULT;
	}
    return status;
}

static int lsm6ds032_release(struct inode *inode, struct file *filp) {
	struct lsm_data *lsm;

	lsm = filp->private_data;

	kfree(lsm->tx);
	lsm->tx = NULL;

	kfree(lsm->rx);
	lsm->rx = NULL;
	return 0;
}

static const struct file_operations lsm_fops = {
	.owner =	THIS_MODULE,
	.write =	lsm6ds032_write,
	.read =		lsm6ds032_read,
	.unlocked_ioctl = lsm6ds032_ioctl,
	.open =		lsm6ds032_open,
	.release =	lsm6ds032_release,
	.llseek =	no_llseek,
};

static const struct spi_device_id lsm6ds032_spi_ids[] = {
	{ .name = DEV_NAME },
	{},
};
MODULE_DEVICE_TABLE(spi, lsm6ds032_spi_ids);

#ifdef CONFIG_OF
static const struct of_device_id lsm6ds032_dt_ids[] = {
	{ .compatible = MAN_DEV_NAME },
	{},
};
MODULE_DEVICE_TABLE(of, lsm6ds032_dt_ids);
#endif

static inline int lsm6ds032_who_am_i(struct spi_device *spi_dev) {
	int status;
	char tx_buf[BUFSIZE];
	char rx_buf[BUFSIZE];

	tx_buf[0] = LSM6DS032_WHO_AM_I_ADDR | DIR_BIT;

	// status = spi_sync_transfer(spi_dev, msg, 1);
	// check the WHO_AM_I register before proceeding
	status = spi_write_then_read(spi_dev, tx_buf, 1, rx_buf, 1);
	if (status < 0) {
		return status;
	}
	if (rx_buf[0] != LSM6DS032_WHO_AM_I) {
		pr_err("WHO_AM_I check failed. Invalid value: %x: ", rx_buf[0]);
		return -ENODEV;
	}
	return status;
}

static int lsm6ds032_probe(struct spi_device *spi_dev) {
	int				status;
	dev_t			devt;
	struct device 	*dev;
	struct lsm_data *lsm;

	// allocate data for our driver
	lsm = kzalloc(sizeof(struct lsm_data), GFP_KERNEL);
	if (!lsm) {
		return -ENOMEM;
	}

	status = lsm6ds032_who_am_i(spi_dev);
	if (status < 0) {
		pr_err("Check wiring and connections with LSM6DS032.");
		kfree(lsm);
		return status;
	}

	devt = MKDEV(LSM_MAJOR, LSM_FIRST_MINOR);
	dev = device_create(lsm_class, &spi_dev->dev, devt, lsm, DEV_NAME);
	if (IS_ERR(dev)) {
		kfree(lsm);
		return PTR_ERR(dev);
	}

	// initialize our device with the given file operations
	cdev_init(&lsm->char_dev, &lsm_fops);
	lsm->char_dev.owner = THIS_MODULE;
	status = cdev_add(&lsm->char_dev, devt, N_MINORS);
	if (status < 0) {
		device_destroy(lsm_class, devt);
		kfree(lsm);
		return status;
	}

	lsm->spi_dev = spi_dev;
	spi_set_drvdata(spi_dev, lsm);
	return 0;
}

static int lsm6ds032_remove(struct spi_device *spi_dev)
{
	struct lsm_data	*lsm = spi_get_drvdata(spi_dev);
	device_destroy(lsm_class, MKDEV(LSM_MAJOR, LSM_FIRST_MINOR));
	kfree(lsm);
	return 0;
}

static struct spi_driver lsm_spi_driver = {
	.driver = {
		.name =			DEV_NAME,
		.owner = 		THIS_MODULE,
		.of_match_table = lsm6ds032_dt_ids,
	},
	.probe =		lsm6ds032_probe,
	.remove =		lsm6ds032_remove,
	.id_table =		lsm6ds032_spi_ids,
};

static int __init lsm6ds032_init(void) {
	dev_t devt;
    int status;

	pr_info("Initializing lsm6ds032 Device Driver module...");
	devt = MKDEV(LSM_MAJOR, LSM_FIRST_MINOR);
	status = register_chrdev_region(devt, N_MINORS, DEV_NAME);
	if (status < 0) {
		pr_err("Failed to allocate character device numbers.");
		return status;
	}

	lsm_class = class_create(THIS_MODULE, DEV_NAME);
	if (IS_ERR(lsm_class)) {
		pr_err("Failed to create lsm6ds032 class.");
		unregister_chrdev_region(devt, N_MINORS);
		return PTR_ERR(lsm_class);
	}

	status = spi_register_driver(&lsm_spi_driver);
	if (status < 0) {
		unregister_chrdev_region(devt, N_MINORS);
		class_destroy(lsm_class);
		pr_err("Failed to register driver.");
		return status;
	}

    pr_info("lsm6ds032 Device Driver initialized.");
    return status;
}

static void __exit lsm6ds032_exit(void) {
	dev_t devt;
    pr_info("Removing lsm6ds032 Device Driver module...");
	spi_unregister_driver(&lsm_spi_driver);
	class_destroy(lsm_class);

	devt = MKDEV(LSM_MAJOR, LSM_FIRST_MINOR);
	unregister_chrdev_region(devt, N_MINORS);
    pr_info("lsm6ds032 Device Driver removed.");
}

module_init(lsm6ds032_init);
module_exit(lsm6ds032_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("LSM6DS032 Device Driver");
MODULE_AUTHOR("sailedeer, sailedeer11@gmail.com");
