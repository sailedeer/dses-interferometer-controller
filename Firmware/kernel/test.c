#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

#include "lsm6ds032.h"
#include "as5048.h"

static inline void print_imu_data(struct motion_data md) {
    float acc_mag;
    float angle;

    printf("Acceleration:\n");
    printf("\tx: %d\n", md.accel.x);
    printf("\ty: %d\n", md.accel.y);
    printf("\tz: %d\n", md.accel.z);

    printf("Gyro:\n");
    printf("\tx: %d\n", md.gyro.x);
    printf("\ty: %d\n", md.gyro.y);
    printf("\tz: %d\n", md.gyro.z);

    acc_mag = sqrtf(powf(md.accel.x, 2) + powf(md.accel.y, 2) + powf(md.accel.z, 2));
    angle = atan2f(-md.accel.x, -md.accel.y) * (180.0f / M_PI);

    printf("Acceleration magnitude: %f\n", acc_mag);
    printf("Acceleration angle: %f\n", angle);
}

static inline void print_enc_data(struct position_data pd) {
    printf("Rotation magnitude: %d\n", pd.mag & ~(0xC0));
    printf("Rotation angle: %d\n", pd.angle & ~(0xC0));
}

int main(int argc, char **argv) {
    int fd_lsm, fd_as;
    ssize_t res;
    struct motion_data md;
    struct position_data pd;

    fd_lsm = open("/dev/lsm6ds032", O_RDWR);

    if (fd_lsm < 0) {
        perror("failed to open device file.");
        return EXIT_FAILURE;
    }

    fd_as = open("/dev/as5048", O_RDWR);

    if (fd_as < 0) {
        perror("failed to open device file.");
        close(fd_lsm);
        return EXIT_FAILURE;
    }

    res = read(fd_lsm, &md, sizeof(struct motion_data));
    if (res < 0) {
        printf("Result of read: %d\n", res);
        close(fd_lsm);
        close(fd_as);
        return EXIT_FAILURE;
    }

    res = read(fd_as, &pd, sizeof(struct position_data));
    if (res < 0) {
        printf("Result of read: %d\n", res);
        close(fd_as);
        close(fd_lsm);
        return EXIT_FAILURE;
    }

    print_imu_data(md);
    print_enc_data(pd);

    close(fd_lsm);
    close(fd_as);
    return EXIT_SUCCESS;
}