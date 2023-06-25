#Demo for the controller and IMU encoder

#import the IMU and motor drivers
import sys
sys.path.append("../drivers/")
from LSM6DSO32 import LSM6DSO32
from TB67H303 import TB67H303

import numpy as np
import time

encoder = LSM6DSO32(0)
elevation = TB67H303(4, 27, 22, 13, 19, 16)

print("Motion Control Demo")
target = float(input("Target Angle (25-75 degrees): "))
print(f"Moving to {target} degrees")

elevation.coast() #don't want to start in the wrong direction
elevation.startMotor(25)

#bang-bang controller
while(True):
    #read encoder position
    ac = encoder.acceleration()

    #calculate direction of gravity vector
    angle = np.arctan2(-ac[0], -ac[1]) * 180/np.pi

    print(f"Current Angle: {angle} degrees")

    if(angle - target > 0.5):
        elevation.setDir(0)
    elif(angle - target < -0.5):
        elevation.setDir(1)
    else:
        elevation.coast() #It's a worm gear so no holding torque is required

    time.sleep(0.2)
