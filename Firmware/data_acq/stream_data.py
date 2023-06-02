import time
import serial
import numpy as np

import sys
sys.path.append("../drivers/")
from LSM6DSO32 import LSM6DSO32

dut = LSM6DSO32(0)

ser = serial.Serial('/dev/ttyACM0', 115200, timeout = 1)
time.sleep(0.5)

ser.write(b'pp')
time.sleep(0.1)

while(True):
    ser.write(b'p')
    ac = dut.acceleration()
    pot = int(ser.readline().strip().decode('utf-8'))
    print("%f, \t %f, \t %f, \t %f, \t %d" % (time.time(), ac[0], ac[1], ac[2], pot ))
    time.sleep(0.01)

