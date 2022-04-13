#ifndef _LSM_HEADER
#define _LSM_HEADER

#include <linux/ioctl.h>

struct vector3d {
	signed short x;
	signed short y;
	signed short z;
};

struct motion_data {
	struct vector3d		gyro;
	struct vector3d		accel;
};

#define LSM_MAGIC				    'L'

#define LSM_R_ACCEL                 _IOR(LSM_MAGIC, 1, struct vector3d)

#define LSM_R_GYRO                  _IOR(LSM_MAGIC, 2, struct vector3d)

#define LSM_R_REG                   _IOR(LSM_MAGIC, 3, unsigned char)
#define LSM_W_REG                   _IOW(LSM_MAGIC, 3, unsigned char)

#endif  // _LSM_HEADER