/*
 *                                     ATtiny85
 *                                  -------u-------
 *              RST - A0 - (D 5) --| 1 PB5   VCC 8 |-- +5V
 *                                 |               |
 *                                 |               |
 *      Relay     <-- A3 - (D 3) ==| 2 PB3   PB2 7 |-- (D 2) - A1  --> Voltage Divider
 *(turn 4-wire fan on/off)         |               |  (made of 2k resistor and 5k B3470 thermistor)
 *                                 |               |
 *                    A2 - (D 4) ==| 3 PB4   PB1 6 |-- (D 1) - PWM --> Fan PWM
 *                                 |               |  (connect to fan pwm pin or a logic-level mosfet)
 *                                 |               |      
 *                          Gnd ---| 4 GND   PB0 5 |-- (D 0) - PWM --> LED
 *                                 -----------------
 *                                 
 */


// normal delay() won't work anymore because we are changing Timer1 behavior
// Adds delay_ms and delay_us functions
#include <util/delay.h>

// Clock at 8mHz
//#define F_CPU 8000000  // This is used by delay.h library
//#define PWMTOP 39
//#define PWMBOTTOM 19

// Clock at 16mHz
#define F_CPU 16000000  // This is used by delay.h library
#define PWMTOP 79
 #define PWMBOTTOM 39

// Temperature (vqoltage divider made of 2k resistor and 5k B3470 thermistor)
// 35C 400 | 40C 412 | 50C 500

// turn off voltage 35C
#define VOL_OFF 400
// turn on voltage 40C
#define VOL_ON 412
// max voltage 50C
#define VOL_MAX 500

// PWM signal, Only works with Pin 1(PB1)
#define PWMPin 1
// PB2 get analog input
#define PotPin A1
// PB3 Enable PWM Fan via relay
#define EnPin 3
// PB0 LED
#define LEDPin 0

int FanOn = 0;
int FanMax = 0;
int LedStat = 0;

void setup()
{
  pinMode(PWMPin, OUTPUT);
  pinMode(EnPin, OUTPUT);
  pinMode(LEDPin, OUTPUT);
  led_off();
}

void loop()
{
  int in;
  
  in = analogRead(PotPin);
      
  if (in > VOL_ON || (in > VOL_OFF && FanOn))
  {
    if (!FanOn)
    {
      FanOn = 1;
      enable_pwm();
    }
    if (in < VOL_MAX)
    {
      if (FanMax)
      {
        FanMax = 0;
        enable_pwm();
      }
      OCR0B = map(in, VOL_OFF, VOL_MAX, PWMBOTTOM, PWMTOP);
      led_on();
    }
    else
    {
      disable_pwm(1);
      FanMax = 1;
      led_toggle();
    }
  }
  else if (FanOn)
  {
    FanOn = 0;
    disable_pwm(0);
    led_off();
  }
  
  _delay_ms(200);
}

void disable_pwm(int mode)
{
  if (mode)
  {
    digitalWrite(EnPin, HIGH);
    _delay_ms(100);
    digitalWrite(PWMPin, HIGH);
  }
  else
  {
    digitalWrite(PWMPin, LOW);
    digitalWrite(EnPin, LOW);
  }
}

void enable_pwm()
{
  digitalWrite(EnPin, HIGH);
  disable_pwm(1);
  _delay_ms(1000);
  // Fast PWM Mode, Prescaler = /8
  // PWM on Pin 1(PB1), Pin 0(PB0) disabled
  // 8Mhz / 8 / (39 + 1) = 25Khz
  TCCR0A = _BV(COM0B1) | _BV(WGM01) | _BV(WGM00);
  TCCR0B = _BV(WGM02) | _BV(CS01);
  //TCCR0B = _BV(WGM02) | _BV(CS02);
  // Set TOP and initialize duty cycle to zero(0)
  OCR0A = PWMTOP;    // TOP - DO NOT CHANGE, SETS PWM PULSE RATE
  OCR0B = PWMBOTTOM;     // duty cycle for Pin 1(PB1) - generates 1 500nS pulse even when 0
}

void led_on()
{
  LedStat = 1;
  digitalWrite(LEDPin, HIGH);
}

void led_off()
{
  LedStat = 0;
  digitalWrite(LEDPin, LOW);
}

void led_toggle()
{
  if (LedStat)
    led_off();
  else
    led_on();
}
