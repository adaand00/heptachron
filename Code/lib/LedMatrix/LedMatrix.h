#ifndef LEDMATRIX_LEDMATRIX_H_
#define LEDMATRIX_LEDMATRIX_H_
#include "Arduino.h"
#include "Wire.h"

class LedMatrix
{
private:
  TwoWire * bus_;
  static constexpr char ADDR_ = 0x34;
  void writeRegister(uint8_t reg, uint8_t val);
  uint8_t readRegister(uint8_t reg);
  
  // Write bytes to switch column (digit)
  void writeByte(char byte, uint8_t sw);

  static constexpr char num_to_seg[] = {
  //0bGFEDCBA
    0b0111111,
    0b0000110,
    0b1011011,
    0b1001111,
    0b1100110,
    0b1101101,
    0b1111101,
    0b0000111,
    0b1111111,
    0b1100111
  };

public:
  LedMatrix(TwoWire * i2c_bus);

  void begin();

  void ShowTime(uint8_t hours, uint8_t  minutes, uint8_t second);
};

#endif /* LEDMATRIX_LEDMATRIX_H_ */
