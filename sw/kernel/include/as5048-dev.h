#ifndef _AS5048_DEV_H
#define _AS5048_DEV_H

#include <linux/types.h>
#include <linux/ioctl.h>

#include <as5048.h>

#define AS5048_MAGIC			'A'

#define AS5048_R_REG            _IOWR(AS5048_MAGIC, 1, __u16)
#define AS5048_W_REG            _IOW(AS5048_MAGIC, 1, __u32)

#define MAN_DEV_NAME			"asm,as5048"
#define DEV_NAME				"as5048"

/*
*   This major number falls into the local/experimental range
*   as specified by the LANANA Device List here:
*   http://mirrors.mit.edu/kernel/linux/docs/lanana/device-list/devices-2.6.txt
*/
#define AS_MAJOR				60
#define AS_FIRST_MINOR			0
#define N_MINORS				1

#define BUFSIZE					8

#define DIR_BIT					BIT(6)

#define MSB_16(addr)			((addr & 0xFF00) >> 8)
#define LSB_16(addr)			(addr & 0xFF)

#endif  // _AS5048_H
