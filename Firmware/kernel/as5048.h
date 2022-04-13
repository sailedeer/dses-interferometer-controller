#ifndef _AS_HEADER
#define _AS_HEADER

#include <linux/ioctl.h>

struct position_data {
	unsigned short      mag;
    unsigned short      angle;
};

#define AS_MAGIC				    'A'

#define AS_R_REG                    _IOR(AS_MAGIC, 1, unsigned short)
#define AS_W_REG                    _IOW(AS_MAGIC, 1, unsigned short)

#endif  // _AS_HEADER