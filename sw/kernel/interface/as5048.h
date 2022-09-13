#ifndef _AS5048_H
#define _AS5048_H

#include <cstdint>

struct position_data {
	uint16_t      mag;
    uint16_t      angle;
};

struct as5048_reg_io {
    uint16_t addr;
    uint16_t value;
};

#endif  // _AS5048_H
