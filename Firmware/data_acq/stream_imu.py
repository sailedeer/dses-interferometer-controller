#Test script for the IMU

import numpy as np
import time

#import the IMU driver
import sys
sys.path.append("../drivers/") #IMU directory
from LSM6DSO32 import LSM6DSO32

dut = LSM6DSO32(0) #CS0
ac = dut.acceleration()

while(True):
    ac = dut.acceleration()
    print("%f \t %f \t %f" % (ac[0], ac[1], ac[2]))
    time.sleep(0.01);
    #print(np.sqrt(ac[0]**2 + ac[1]**2 + ac[2]**2))
    #print(np.arctan2(-ac[0], -ac[1]) * 180/np.pi)
