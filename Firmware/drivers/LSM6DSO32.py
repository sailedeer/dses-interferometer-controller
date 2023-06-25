#driver class for the LSM6DSO32

#import required libraries
import spidev
import RPi.GPIO as GPIO

class LSM6DSO32:

    bus = 0

    # Register definitions
    FUNC_CFG_ACCESS = 0x01
    PIN_CTRL = 0x02
    FIFO_CTRL1 = 0x07
    FIFO_CTRL2 = 0x08
    FIFO_CTRL3 = 0x09
    FIFO_CTRL4 = 0x0A
    COUNTER_BDR_REG1 = 0x0B
    COUNTER_BDR_REG2 = 0x0C
    INT1_CTRL = 0x0D
    INT2_CTRL = 0x0E
    WHO_AM_I = 0x0F
    CTRL1_XL = 0x10
    CTRL2_G = 0x11
    CTRL3_C = 0x12
    CTRL4_C = 0x13
    CTRL5_C = 0x14
    CTRL6_C = 0x15
    CTRL7_G = 0x16
    CTRL8_XL = 0x17
    CTRL9_XL = 0x18
    CTRL10_C = 0x19
    ALL_INT_SRC = 0x1A
    WAKE_UP_SRC = 0x1B
    TAP_SRC = 0x1C
    D6D_SRC = 0x1D
    STATUS_REG = 0x1E
    OUT_TEMP_L = 0x20
    OUT_TEMP_H = 0x21
    OUTX_L_G = 0x22
    OUTX_H_G = 0x23
    OUTY_L_G = 0x24
    OUTY_H_G = 0x25
    OUTZ_L_G = 0x26
    OUTZ_H_G = 0x27
    OUTX_L_A = 0x28
    OUTX_H_A = 0x29
    OUTY_L_A = 0x2A
    OUTY_H_A = 0x2B
    OUTZ_L_A = 0x2C
    OUTZ_H_A = 0x2D
    EMB_FUNC_STATUS_MAINPAGE = 0x35
    FSM_STATUS_A_MAINPAGE = 0x36
    STATUS_MASTER_MAINPAGE = 0x39
    FIFO_STATUS1 = 0x3A
    FIFO_STATUS2 = 0x3B
    TIMESTAMP0 = 0x40
    TIMESTAMP1 = 0x41
    TIMESTAMP2 = 0x42
    TIMESTAMP3 = 0x43
    TAP_CFG0 = 0x56
    TAP_CFG1 = 0x57
    TAP_CFG2 = 0x58
    TAP_THS_6D = 0x59
    INT_DUR2 = 0x5A
    WAKE_UP_THS = 0x5B
    WAKE_UP_DUR = 0x5C
    FREE_FALL = 0x5D
    MD1_CFG = 0x5E
    MD2_CFG = 0x5F
    I3C_BUS_AVB = 0x62
    INTERNAL_FREQ_FINE = 0x63
    X_OFS_USR = 0x73
    Y_OFS_USR = 0x74
    Z_OFS_USR = 0x75
    FIFO_DATA_OUT_TAG = 0x78
    FIFO_DATA_OUT_X_L = 0x79
    FIFO_DATA_OUT_X_H = 0x7A
    FIFO_DATA_OUT_Y_L = 0x7B
    FIFO_DATA_OUT_Y_H = 0x7C
    FIFO_DATA_OUT_Z_L = 0x7D
    FIFO_DATA_OUT_Z_H = 0x7E
    
    def readReg(self, reg):
        msg = [reg | 0x80, 0x00]
        return self.spi.xfer2(msg)[1]

    def writeReg(self, reg, value):
        msg = [reg, value]
        self.spi.xfer2(msg)

    #Converts an int that contains a signed int16 (but doesn't know it) into an int that does
    @staticmethod
    def sint16_to_int(value):
        if(value & 0x8000): #value is negative
            return -(2**16 - value)
        else: 
            return value
   
    def acceleration(self):

        #wait for acceleration data to become available
        while not (self.readReg(self.STATUS_REG) & 0x01):
            pass

        #read the acceleration data
        x = self.sint16_to_int(self.readReg(self.OUTX_H_A) << 8 | 
                self.readReg(self.OUTX_L_A))/2**13
        y = self.sint16_to_int(self.readReg(self.OUTY_H_A) << 8 | 
                self.readReg(self.OUTY_L_A))/2**13
        z = self.sint16_to_int(self.readReg(self.OUTZ_H_A) << 8 | 
                self.readReg(self.OUTZ_L_A))/2**13
        return [x, y, z]

    def __init__(self, cs):

        #enable the encoder buffers
        GPIO.setmode(GPIO.BCM)
        GPIO.setup(20, GPIO.OUT)
        GPIO.output(20, GPIO.LOW)

        self.cs = cs
        self.spi = spidev.SpiDev()
        self.spi.open(self.bus, cs)

        #configure SPI
        self.spi.max_speed_hz = 200000
        self.spi.mode = 0

        #check to see if the device is communicating properly
        if(self.readReg(self.WHO_AM_I) != 0x6C):
            raise Exception("Failed to read expected ID from WHO_AM_I register. Check wiring and connections")

        #104Hz ODR, +-4g for accelerometer, enable LPF2 digital filtering
        self.writeReg(self.CTRL1_XL, 0b01000000)

        #104Hz ODR, +-125dp for gyroscope
        self.writeReg(self.CTRL2_G, 0b01000010)

        #All interrupt signals on INT1 pin, DRDY_MASK on, Gyro LPF1 on
        self.writeReg(self.CTRL4_C, 0b00101010)

        #Set gyroscope LPF1 bandwidth to 11.5Hz
        self.writeReg(self.CTRL6_C, 0b00000111)

        #With CTRL8_XL = 0x00, LPF bandwidth is ODR/4


    def __del__(self):
        GPIO.cleanup()
