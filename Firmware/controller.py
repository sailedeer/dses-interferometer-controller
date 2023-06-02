#Demo for the controller and IMU encoder

#import the IMU and motor drivers
import sys
import argparse
sys.path.append(".")
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
ki = 0.001
kp = 0.01

if(args.s):
    filename =  f"{args.target}-{kp}-{ki}-{period}-{time.time()}".replace('.', '_')
    f = open(datapath + filename + ".csv", "a")
    #f.write("target: {target}\n"
    #        "kp: {kp}\n"
    #        "ki: {ki}\n"
    #        "period: {period}")
        
def moveto(target):
    p = 0
    i = 0

    while(True):
        angle = get_angle()

        error = target - angle
        p = error
        i += error

        if(abs(error) < 0.1):
            elevation.coast()
            return
    
        u = np.clip((p*kp + i*ki*period), -1, 1)
        elevation.command(u)

        time.sleep(period)

moveto(args.target)
f.close()
