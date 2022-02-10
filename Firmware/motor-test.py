#test for the motor driver class

#import the motor driver
import sys
sys.path.append(".") #motor driver directory
from TB67H303 import TB67H303

#create a motor driver object
dut = TB67H303(4, 27, 22, 13, 19, 16)

dut.drive(1, 25)
