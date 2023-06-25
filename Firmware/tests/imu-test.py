#Test script for the IMU

import numpy as np

#import the IMU driver
import sys
sys.path.append("../drivers/") #IMU directory
from LSM6DSO32 import LSM6DSO32

dut = LSM6DSO32(0) #CS0
ac = dut.acceleration()

print(ac)
print(np.sqrt(ac[0]**2 + ac[1]**2 + ac[2]**2))
print(np.arctan2(-ac[0], -ac[1]) * 180/np.pi)
