#Demo for the controller and IMU encoder

#import the IMU and motor drivers
import sys
import argparse
sys.path.append("../drivers/")
from LSM6DSO32 import LSM6DSO32
from TB67H303 import TB67H303

import numpy as np
import time

#Handle input arguments
parser = argparse.ArgumentParser(description="Drive the dish to a specific direction.")
parser.add_argument('target', type=int, help='Angle from horizontal to point the dish')
parser.add_argument('-s', '-save_output', help='Save variables to .csv file', action='store_true')

args = parser.parse_args()

datapath = "./data/"

encoder = LSM6DSO32(0)
elevation = TB67H303(17, 5, 6, 26, 21, 12)

def get_angle():
    ac = encoder.acceleration()
    if(args.s):
        f.write(f"{ac[0]}, {ac[1]}\n")
    return np.arctan2(ac[1], ac[0]) * 180/np.pi

elevation.coast() #don't want to start in the wrong direction

period = 0.01
amax = 4 #(degrees/s/s)
full_speed_rate = 4.8 #(degrees/second @100%)
max_stop_speed = 20 #(percent)

_maxstep = period*amax/full_speed_rate

if(args.s):
    filename =  f"{args.target}-{kp}-{ki}-{period}-{time.time()}".replace('.', '_')
    f = open(datapath + filename + ".csv", "a")
        
def moveto(target):
    u = 0
    u_last = 0

    slow_flag = False #for when we approach the target, force dish to slow down

    while(True):
        angle = get_angle()

        error = target - angle

        if(abs(error) < 0.1):
            elevation.coast()
            return
        
        u_last = u
        u = 1 if (error > 0) else -1

        #ramp down
        speed = u_last * full_speed_rate
        slowing_time = (abs(speed)-max_stop_speed*full_speed_rate/100)/amax
        if(abs(error) < 0.5*abs(speed)*slowing_time):
            slow_flag = True #latch this on

        if(slow_flag):
            u = u * max_stop_speed/100
       
        #enforce an acceleration limit:
        if(u - u_last > _maxstep):
            u = u_last + _maxstep
        elif(u_last - u > _maxstep):
            u = u_last - _maxstep

        elevation.command(u)

        time.sleep(period)

moveto(args.target)

if(args.s):
    f.close()
