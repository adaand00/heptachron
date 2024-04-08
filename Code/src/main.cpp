#include <Arduino.h>
#include "Wire.h"

#include "LedMatrix.h"
#include "RV-3028-C7.h"

#define SCL PIN_PB0
#define SDA PIN_PB1

#define LED_EN PIN_PB2

#define RTC_CLK PIN_PB3

#define TX PIN_PA1
#define RX PIN_PA2

#define LIGHT_IN PIN_PA3
#define LIGHT_EN PIN_PA4

#define PGOOD PIN_PA5

#define BU1 PIN_PA6
#define BU2 PIN_PA7

#define LED_ADDR 0x34
#define RTK_ADDR 0x52

LedMatrix mat(&Wire);
RV3028 rtc;

int i = 0;

void setup() {
  delay(100);
  pinMode(LED_EN, OUTPUT);
  digitalWrite(LED_EN, HIGH);

  // put your setup code here, to run once:
  Wire.begin();

  mat.begin();
  
  // Allow for RTC to init
  delay(100);

  rtc.begin(Wire, true, true, false);
  rtc.setToCompilerTime();

  rtc.enableClockOut(FD_CLKOUT_1);

  rtc.updateTime();

  mat.ShowTime(rtc.getHours(), rtc.getMinutes(), rtc.getSeconds());

}

void loop() {
  mat.ShowTime(i*10+i,i*10+i,i);
  i = (i+1)%10;
  // put your main code here, to run repeatedly:
  delay(1000);
  
}
