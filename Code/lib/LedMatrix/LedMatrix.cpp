#include "LedMatrix.h"

LedMatrix::LedMatrix(TwoWire * i2c_bus){
  bus_ = i2c_bus;
}

void LedMatrix::begin(){
  bus_->begin();

  //Scaling of all current sources (CSy)
  for (int i = 0; i < 16; i++)
  {
    writeRegister(0x90 + i, 64);
  }
  
  //Global current, max 64
  writeRegister(0xA1, 5);

  //Conf register, 5 SW, 2.4V Logic, disable open/short det, Normal operation
  writeRegister(0xA0, 0b01001001); 

}

void LedMatrix::writeByte(char byte, uint8_t sw){
  char start = sw*16 + 1;
  for(int i = start; i < start + 8; i++){
    writeRegister(i, (byte & 1) * 255);
    byte = byte >> 1;
  }

}

void LedMatrix::ShowTime(uint8_t h, uint8_t m, uint8_t s){

  writeByte(num_to_seg[m%10], 0);
  writeByte(num_to_seg[(m/10)%10], 1);
 
  writeByte(num_to_seg[h%10], 2);
  writeByte(num_to_seg[(h/10)%10], 3);

}

void LedMatrix::writeRegister(uint8_t reg, uint8_t val){

  bus_->beginTransmission(this->ADDR_);
  bus_->write(reg);
  bus_->write(val);
  bus_->endTransmission();

}

uint8_t LedMatrix::readRegister(uint8_t reg){

  uint8_t result;

  bus_->beginTransmission(this->ADDR_);
  bus_->write(reg);
  bus_->endTransmission();

  bus_->requestFrom(this->ADDR_, 1);

  while (bus_->available())
  {
    result = bus_->read();
  }

}