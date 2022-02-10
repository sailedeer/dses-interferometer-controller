#TB67H303 driver class

#import required libraries
import RPi.GPIO as GPIO

class TB67H303:

    #Functionally similar to coast, but does not depend on IN1/IN2
    def standby(self):
        GPIO.output(self.stby, GPIO.LOW)
        self.pwm_obj.stop()

    #Move the motor one way (direction = 1) or the other (direction = 0
    #Output PWM is set by speed, between 0 and 100.0
    def drive(self, direction, speed):
        self.pwm_obj.start(speed)

        if (direction):
            GPIO.output(self.in1, GPIO.LOW)
            GPIO.output(self.in2, GPIO.HIGH)
        else:
            GPIO.output(self.in2, GPIO.LOW)
            GPIO.output(self.in1, GPIO.HIGH)

        GPIO.output(self.stby, GPIO.HIGH)

    #allow motor to come to a stop: no holding force
    def coast(self):
        GPIO.output(self.in1, GPIO.LOW)
        GPIO.output(self.in2, GPIO.LOW)
        GPIO.output(self.stby, GPIO.HIGH)

    #stop the motor and hold in in place
    def brake(self):
        GPIO.output(self.in1, GPIO.HIGH)
        GPIO.output(self.in2, GPIO.HIGH)
        GPIO.output(self.stby, GPIO.HIGH)

    #return the state of the TSD/ISD alert output
    def get_tsd_isd(self):
        return GPIO.input(self.a1)
    
    #return the state of the UVLO alert output
    def get_uvlo(self):
        return GPIO.input(self.a2)

    #Constructor
    def __init__(self, stby, in1, in2, a1, a2, pwm):
        
        #save pin definitions to private variables
        self.stby = stby
        self.in1 = in1
        self.in2 = in2
        self.a1 = a1
        self.a2 = a2
        self.pwm = pwm

        #configure GPIO pins for TB67H303
        GPIO.setmode(GPIO.BCM)
        GPIO.setup(stby, GPIO.OUT)
        GPIO.setup(in1, GPIO.OUT)
        GPIO.setup(in2, GPIO.OUT)
        GPIO.setup(a1, GPIO.IN)
        GPIO.setup(a2, GPIO.IN)
        GPIO.setup(pwm, GPIO.OUT)

        #set up PWM
        self.pwm_obj = GPIO.PWM(pwm, 10000) #PWM Frequency is 10kHz

        #initial state
        self.standby()

    #Destructor
    def __del__(self):
        #Get rid of GPIO assignments
        GPIO.cleanup()

