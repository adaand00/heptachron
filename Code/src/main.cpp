#include <Arduino.h>
#include "Wire.h"
#include "avr/sleep.h"

#include "LedMatrix.h"
#include "RV3028C7.h"

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
//RV3028 rtc;
RV3028C7 rtc;

int i = 0;

void setup() {
  // Setup sleep mode
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  
  // Setup pins
  pinMode(LIGHT_EN, OUTPUT);
  digitalWrite(LIGHT_EN, LOW);
  pinMode(LIGHT_IN, INPUT);
  pinMode(BU1, INPUT_PULLUP);
  pinMode(BU2, INPUT_PULLUP);

  pinMode(LED_EN, OUTPUT);
  digitalWrite(LED_EN, HIGH);

  // Clock pin interrupts
  pinMode(RTC_CLK, INPUT);
  PORTB_PIN3CTRL |= PORT_ISC_BOTHEDGES_gc;

  // I2C 
  Wire.begin();
  Wire.setClock(400000);

  // LED matrix driver
  mat.begin();

  delay(100);

  // RTC 
  rtc.begin(Wire);
  delay(1000);
  rtc.disableClockOutput();
  delay(1000);
  rtc.enableClockOutput(CLKOUT_1HZ);
}

ISR(PORTB_PORT_vect){
  VPORTB_INTFLAGS |= 1<<3;
}

void loop() {
  // Waits until pin change interrupt
  // Either RTC tick or button press
  sleep_cpu(); 

  // TODO: handle button inputs

  // TODO: read battery voltage

  // Read phototransistor
  digitalWrite(LIGHT_EN, HIGH);
  delay(2); 
  int l = analogRead(LIGHT_IN);
  digitalWrite(LIGHT_EN, LOW);

  // Read time and update LEDs
  rtc.updateTime();
  uint8_t h = rtc.getDateTimeComponent(DATETIME_HOUR);
  uint8_t m = rtc.getDateTimeComponent(DATETIME_MINUTE);
  uint8_t s = rtc.getDateTimeComponent(DATETIME_SECOND);
  mat.ShowTime(h, m, s); 
}
