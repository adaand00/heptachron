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

typedef struct button
{
  bool pressed;
  unsigned long time;
} button_t;


button_t b1;
button_t b2;

int i = 50;

void setup() {
  // Setup sleep mode
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  
  // Setup pins
  pinMode(LIGHT_EN, OUTPUT);
  digitalWrite(LIGHT_EN, LOW);
  pinMode(LIGHT_IN, INPUT);

  pinMode(BU1, INPUT_PULLUP);
  PORTA_PIN6CTRL |= PORT_ISC_BOTHEDGES_gc;

  pinMode(BU2, INPUT_PULLUP);
  PORTA_PIN7CTRL |= PORT_ISC_BOTHEDGES_gc;

  pinMode(LED_EN, OUTPUT);
  digitalWrite(LED_EN, HIGH);

  pinMode(PGOOD, INPUT_PULLUP);
  pinMode(RX, INPUT_PULLUP);
  pinMode(TX, INPUT_PULLUP);

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

ISR(PORTA_PORT_vect){
  int f = VPORTA_INTFLAGS;

 
  if(f == 1<<6){
    // Button 1
    VPORTA_INTFLAGS |= 1<<6;
    if (!digitalReadFast(BU1)){
      b1.pressed = true;
      b1.time = millis();
    }else{
      b1.pressed = false;
    }
  }else if(f == 1<<7){
    // Button 2
    VPORTA_INTFLAGS |= 1<<7;
    if (!digitalReadFast(BU2)){
      b2.pressed = true;
      b2.time = millis();
    }else{
      b2.pressed = false;
    }
  }
}

void loop() {
  // Disable ADC before sleep, brings system from 100 uA to 10 uA deep sleep.
  ADC0.CTRLA &= ~ADC_ENABLE_bm;

  // Waits until pin change interrupt
  // Either RTC tick or button press
  sleep_cpu(); 

  //Enable ADC after being woken up
  ADC0.CTRLA |= ADC_ENABLE_bm;

  int s1 = 0;

  if(b1.pressed){
    digitalWrite(LED_EN, LOW);
    PORTB_PIN3CTRL &= ~PORT_ISC_BOTHEDGES_gc;
  }

  if(b2.pressed){
    digitalWrite(LED_EN, HIGH);
    PORTB_PIN3CTRL |= PORT_ISC_BOTHEDGES_gc;
  }

  // TODO: read battery voltage

  // Start phototransistor 
  digitalWrite(LIGHT_EN, HIGH);

  // Read time and update LEDs
  rtc.updateTime();
  uint8_t h = rtc.getDateTimeComponent(DATETIME_HOUR);
  uint8_t m = rtc.getDateTimeComponent(DATETIME_MINUTE);
  uint8_t s = rtc.getDateTimeComponent(DATETIME_SECOND);
  mat.ShowTime(b2.pressed*10 + s1, m, 0); 


  // Read phototransistor
  int l = analogRead(LIGHT_IN);
  digitalWrite(LIGHT_EN, LOW);
  
}