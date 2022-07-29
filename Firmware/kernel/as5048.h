#ifndef _AS_H
#define _AS_H

#include <linux/types.h>
#include <linux/ioctl.h>

struct position_data {
	unsigned short      mag;
    unsigned short      angle;
};

struct as5048_reg_io {
    unsigned short addr;
    unsigned short value;
};

#define AS_MAGIC				    'A'

#define AS_R_REG                    _IOWR(AS_MAGIC, 1, __u16)
#define AS_W_REG                    _IOW(AS_MAGIC, 1, __u32)

#endif  // _AS_H