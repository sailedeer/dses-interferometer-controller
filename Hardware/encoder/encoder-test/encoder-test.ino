
#include <SPI.h>
#include "AS5048A.h"

#define CSN 10
#define PWM 9

void setup() {
    pinMode(CSN, OUTPUT);
    SPI.begin();
    Serial.begin(9600);
    digitalWrite(CSN, HIGH);
    delay(100);
    Serial.println(readReg(NOP));
    
}

void loop() {
  delay(1000);
  Serial.println(360 - (readReg(ANGLE)-477) / 45.511);

}

uint16_t readReg(uint16_t reg){
  uint16_t payload = reg | (3 << 14);
  SPI.beginTransaction(SPISettings(400000, MSBFIRST, SPI_MODE1));
  digitalWrite(CSN, LOW);
  SPI.transfer16(payload);
  digitalWrite(CSN, HIGH);
  digitalWrite(CSN, LOW);
  uint16_t retval = SPI.transfer16(00);
  digitalWrite(CSN, HIGH);
  SPI.endTransaction();
  return retval & (0x3FFF);
}
