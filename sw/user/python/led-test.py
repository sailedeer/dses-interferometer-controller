# Test for the two user-controllable LEDs on the board

import RPi.GPIO as GPIO

az_led = 15
el_led = 18

GPIO.setmode(GPIO.BCM)

GPIO.setup(az_led, GPIO.OUT)
GPIO.setup(el_led, GPIO.OUT)

p1 = GPIO.PWM(az_led, 0.5)
p2 = GPIO.PWM(el_led, 0.5)

p1.start(50)
p2.start(50)

input('Press return to stop:')
p1.stop()
p2.stop()
GPIO.cleanup()
