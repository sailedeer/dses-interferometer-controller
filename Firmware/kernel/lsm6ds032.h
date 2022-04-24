#ifndef _LSM_HEADER
#define _LSM_HEADER

#include <linux/ioctl.h>

struct vector3d {
	__s16		x;
	__s16		y;
	__s16		z;
};

struct motion_data {
	struct vector3d		gyro;
	struct vector3d		accel;
};

#define LSM_MAGIC				    'L'

#define LSM_R_ACCEL                 _IOR(LSM_MAGIC, 1, struct vector3d)

#define LSM_R_GYRO                  _IOR(LSM_MAGIC, 2, struct vector3d)

#define LSM_R_REG                   _IOR(LSM_MAGIC, 3, __u8)
#define LSM_W_REG                   _IOWR(LSM_MAGIC, 3, __u8)

#endif  // _LSM_HEADER