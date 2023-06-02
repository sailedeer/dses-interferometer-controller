#test for the motor driver class

#import the motor driver
import sys
sys.path.append(".") #motor driver directory
from TB67H303 import TB67H303

#create a motor driver object (azimuth motor on the dish controller PCB)
#dut = TB67H303(4, 27, 22, 13, 19, 16)
dut = TB67H303(17, 5, 6, 26, 21, 12)

print('Driving the azimuth motor at duty-cycle=25 in direction 1')
dut.setDir(1)
dut.startMotor(25)
input('Press return to stop:')

