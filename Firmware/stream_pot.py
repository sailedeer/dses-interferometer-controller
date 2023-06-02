import serial
import time

ser = serial.Serial('/dev/ttyACM0', 115200, timeout = 1)
time.sleep(0.5)

ser.write(b'pppp')
time.sleep(1)
ser.write(b'p')
time.sleep(1)
print("Streaming potentiometer data, Ctrl-C to stop")
while(True):
    ser.write(b'p')
    time.sleep(0.5)
    print(ser.readline().strip().decode("utf-8"))
