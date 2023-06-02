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
parser = argparse.ArgumentParser(description="Test the frequency response over a range of frequencies")
parser.add_argument('start', type=float, help='Starting Frequency')
parser.add_argument('stop', type=float, help='Stop Frequency')
parser.add_argument('numpts', type=int, help='Number of Points')

args = parser.parse_args()

datapath = "./data/"

encoder = LSM6DSO32(0)
elevation = TB67H303(17, 5, 6, 26, 21, 12)

def get_angle():
    ac = encoder.acceleration()
    return np.arctan2(ac[1], ac[0]) * 180/np.pi

elevation.coast() #don't want to start in the wrong direction

period = 0.01

filename =  f"chirp-{args.start}-{args.stop}-{args.numpts}-{period}-{time.time()}".replace('.', '_')
f = open(datapath + filename + ".csv", "a")

exp_len = args.numpts * 3
t = np.arange(0, exp_len, period)
cmds = np.sin(np.pi*((args.stop - args.start)/exp_len*t**2 + 2*args.start*t))

for i in range(len(t)):
    elevation.command(cmds[i])
    angle = get_angle()
    f.write(f"{time.time()}, {angle}, {cmds[i]}/n")

f.close()

