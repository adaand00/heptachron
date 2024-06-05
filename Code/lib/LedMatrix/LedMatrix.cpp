#include "LedMatrix.h"

LedMatrix::LedMatrix(TwoWire * i2c_bus){
  bus_ = i2c_bus;
}

void LedMatrix::begin(){
  bus_->begin();

  //Scaling of all current sources (CSy)
  for (int i = 0; i < 16; i++)
  {
    writeRegister(0x90 + i, 150);
  }
  
  //Global current, max 64, keep very low to maximize battery life
  writeRegister(0xA1, 1);

  //Conf register, 5 SW, 2.4V Logic, disable open/short det, Normal operation
  writeRegister(0xA0, 0b01000001);

  // Pull up/down, 
  writeRegister(0xB0, 0b00000000);

  // PWM frequency register, 1kHZ
  writeRegister(0xB2, 0b00000100);

}

void LedMatrix::writeByte(char byte, uint8_t sw){
  char start = sw*16 + 1;
  for(int i = start; i < start + 8; i++){
    if(sw == 4){
      // Special brightness for small LEDS
      writeRegister(i, (byte & 1) * 255);
    }else{
      writeRegister(i, (byte & 1) * 255);
    }
    byte = byte >> 1;
  }
}

void LedMatrix::setBrightness(uint8_t brightness){
  for (int i= 0; i<16; i++)
  {
    writeRegister(0x90 + i, brightness);
  }

}

void LedMatrix::ShowBytes(
  uint8_t D3, uint8_t D2, uint8_t D1, uint8_t D0, uint8_t b)
{
  writeByte(D3, 3);
  writeByte(D2, 2);
  writeByte(D1, 1);
  writeByte(D0, 0);
  writeByte(b, 4);
}

void LedMatrix::ShowTime(uint8_t h, uint8_t m, uint8_t s, bool c){
  // Turn off pixels with m = 255
  if(m < 99){
    writeByte(num_to_seg[m%10], 0);
    writeByte(num_to_seg[(m/10)%10], 1);
  }else{
    writeByte(0, 0);
    writeByte(0, 1);
  }

  // Turn off pixels with h = 255
  if(h < 99){
    writeByte(num_to_seg[h%10], 2);
    writeByte(num_to_seg[(h/10)%10], 3);
  }else{
    writeByte(0, 2);
    writeByte(0, 3);
  }

  // Reverse bits in s
  uint8_t s_rev = 0;
  for (size_t i = 0; i < 6; i++)
  {
    s_rev |= ((s >> i) & 1) << (5-i);
  }

  s = s_rev;

  if(c){
    // Colon at two top bits
    s = 0b11000000 | s;
  }

  writeByte(s, 4);
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