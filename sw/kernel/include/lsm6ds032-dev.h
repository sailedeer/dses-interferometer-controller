#ifndef _LSM6DS032_DEV_H
#define _LSM6DS032_DEV_H

#include <linux/types.h>
#include <linux/bits.h>
#include <linux/ioctl.h>

#include <lsm6ds032.h>

#define LSM_MAGIC				    'l'
#define LSM_R_REG					_IOWR(LSM_MAGIC, 1, struct lsm6ds032_reg_io *)
#define LSM_W_REG					_IOWR(LSM_MAGIC, 2, struct lsm6ds032_reg_io *)

#define MAN_DEV_NAME			"st,lsm6ds032"
#define DEV_NAME				"lsm6ds032"

/*
*   This major number falls into the local/experimental range
*   as specified by the LANANA Device List here:
*   http://mirrors.mit.edu/kernel/linux/docs/lanana/device-list/devices-2.6.txt
*/
#define LSM_MAJOR				61
#define LSM_FIRST_MINOR			0
#define N_MINORS				1

#define BUFSIZE					128

#define DIR_BIT					BIT(7)

#endif  // _LSM6DS032_DEV_H
