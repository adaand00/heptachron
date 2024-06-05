#include <Arduino.h>
#include "Wire.h"
#include "avr/sleep.h"

#include "LedMatrix.h"
#include "RV3028C7.h"

#define SCL PIN_PB0
#define SDA PIN_PB1

#define LED_EN PIN_PB2

#define RTC_CLK PIN_PA3
#define RTC_INT PIN_PA5

#define LIGHT_IN PIN_PA1
#define LIGHT_EN PIN_PA2

#define STAT1 PIN_PB3
#define STAT2 PIN_PA4

#define BU1 PIN_PA6
#define BU2 PIN_PA7

#define LED_ADDR 0x34
#define RTK_ADDR 0x52

LedMatrix mat(&Wire);
RV3028C7 rtc;

typedef enum screens {
  TIME,
  BAT,
  // BRIGHTNESS,
  SCREENS_MAX,
} screen_t;

screen_t screen;
uint8_t edit = false;

uint8_t idle_count;

int bat = 0;
int light = 0;
volatile int i = 0;

// Overwrite TCA0 init since we do not use analogWrite(), saves some flash 
void init_TCA0() { };

void setup() {
  // Setup sleep mode
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  
  // Setup pins
  pinMode(LIGHT_EN, OUTPUT);
  digitalWriteFast(LIGHT_EN, LOW);
  pinMode(LIGHT_IN, INPUT);

  pinMode(BU1, INPUT_PULLUP);
  PORTA_PIN6CTRL |= PORT_ISC_BOTHEDGES_gc;

  pinMode(BU2, INPUT_PULLUP);
  PORTA_PIN7CTRL |= PORT_ISC_BOTHEDGES_gc;

  pinMode(LED_EN, OUTPUT);
  digitalWriteFast(LED_EN, HIGH);

  //pinMode(PGOOD, INPUT_PULLUP);
  //pinMode(RX, INPUT_PULLUP);
  //pinMode(TX, INPUT_PULLUP);

  // Clock pin interrupts
  pinMode(RTC_CLK, INPUT);
  PORTA_PIN3CTRL |= PORT_ISC_BOTHEDGES_gc;

  // Set fixed adc reference
  analogReference(INTERNAL1V024);
  // Disable ADC before sleep, brings system from 100 uA to 10 uA deep sleep.
  ADC0.CTRLA &= ~ADC_ENABLE_bm;

  // I2C 
  Wire.begin();
  Wire.setClock(400000);

  // LED matrix driver
  mat.begin();

  // RTC 
  rtc.begin(Wire);
  rtc.disableClockOutput();
  delay(1000);
  rtc.enableClockOutput(CLKOUT_1HZ);

  // init state
  screen = TIME;
}

ISR(PORTA_PORT_vect){
  int f = VPORTA_INTFLAGS;

  if(f == 1<<6){
    // Button 1
    VPORTA_INTFLAGS |= 1<<6;
  }
  if(f == 1<<7){
    // Button 2
    VPORTA_INTFLAGS |= 1<<7;
  }
  if(f == 1<<3){
    // RTC 
    i++;
    VPORTA_INTFLAGS |= 1<<3;
  }
}

void loop() {
  // Waits until pin change interrupt
  // Either RTC tick or button press
  sleep_cpu(); 
  // Increment wakeup count

  
  //Enable ADC after being woken up
  ADC0.CTRLA |= ADC_ENABLE_bm;
  // Read battery voltage, int v = 1024*VDD/10
  bat = analogRead(ADC_VDDDIV10);

  /*
  if(bat < 370){
    state = BUVLO;
  }
  */

  // Start phototransistor 
  digitalWriteFast(LIGHT_EN, HIGH);
  delay(2);

  // Read phototransistor
  light = analogRead(LIGHT_IN);
  digitalWriteFast(LIGHT_EN, LOW);
  // Turn off ADC
  ADC0.CTRLA &= ~ADC_ENABLE_bm;

  //mat.ShowBytes(255, 255, 255, 255, 255);
  
  // Read buttons
  bool b1 = !digitalReadFast(BU1);
  bool b2 = !digitalReadFast(BU2);

  // b1 goes to next screen;
  if(!edit && b1){
    screen = (screen_t) ((screen+1) % SCREENS_MAX);
  }

  switch (screen)
  {
  case TIME:
    // Read time and update LEDs
    rtc.updateTime();
    uint8_t h = rtc.getDateTimeComponent(DATETIME_HOUR);
    uint8_t m = rtc.getDateTimeComponent(DATETIME_MINUTE);
    uint8_t s = rtc.getDateTimeComponent(DATETIME_SECOND);

    // Go to next edit
    if(b2){
      edit = (edit+1)%3;
    }
    
    if(edit==1){
      // Edit minutes
      m=(m+b1)%60;
      rtc.setDateTimeComponent(DATETIME_MINUTE, m);
      rtc.synchronize();
      rtc.setDateTimeComponent(DATETIME_SECOND, 0);
      rtc.synchronize();
      
      // Blink minutes
      if(i%2 == 0){
        m = 255;
      }
      
    }else if(edit==2){
      // Edit hours 
      h=(h+b1)%24;
      rtc.setDateTimeComponent(DATETIME_HOUR, h);
      rtc.synchronize();
      rtc.setDateTimeComponent(DATETIME_SECOND, 0);
      rtc.synchronize();

      if(i%2 == 0){
        h = 255;
      }
    }

    mat.ShowTime(h, m, s, i%2);
    break;

  case BAT:
    uint8_t t = 0;
    mat.ShowBytes(0b01111100,0b01110111,0b01111000,0,t);

    if(b1){
      // Toggle LEDs on/off
      digitalWriteFast(LED_EN, !digitalReadFast(LED_EN));
    }
    break;

  default:
    break;
  }
  


}
