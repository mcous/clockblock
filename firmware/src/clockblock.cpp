// clockblock
// Copyright 2013 by Wiley Cousins, LLC.
// shared under the terms of the MIT License
//
// file: clockblock.cpp
// description: application file for the clockblock

// ************************************
// AVR includes necessary for this file
// ************************************
//#include <util/atomic.h>
#include <util/delay.h>

// ********************
// application includes
// ********************
#include "clockblock.h"


// **************************
// INTERRUPT SERVICE ROUTINES
// **************************
// this ISR driven by a 1024 Hz squarewave from the RTC
ISR(RTC_EXT_INT_vect) {
  // tick the display 32 times a second
  if ( ++ms >= 32) {
    tick = true;
    ms = 0;
  }
}

// ISR for switch inputs
ISR(INPUT_PCINT_vect, ISR_NOBLOCK) {
  buttons.handleChange();
}

// switch debouncer / timer
ISR(INPUT_TIMER_vect, ISR_NOBLOCK) {
  buttons.handleTimer();
}

// ***********
// application
// ***********
// main
int main(void) {
  // delay for a few ms to allow the RTC to take its initial temp measurement
  _delay_ms(1000);

  // begin setup - disable interrupts
  cli();

  // give those ISR volatile vairables some values
  ms = 0;
  tick = false;

  // application variables
  // time vector - { seconds, minutes, hours}
  uint8_t tm[3] = {0, 0, 12};
  // animation frame
  uint8_t fr = 0;
  // arms leds
  uint16_t dots[DISPLAY_NUM_DOTS];

  // initialize the LED driver
  // DO THIS BEFORE RTC (to ensure proper SPI functionality for the RTC)
  tlc.init();
  // set the TLC to autorepeat the pattern and to reset the GS counter whenever new data is latched in
  tlc.setFC(TLC5971_DSPRPT);

  // initialize the RTC
  rtc.init();
  // enable a 1024 Hz squarewave output on interrupt pin
  rtc.setSquareWave(PCF2129AT_CLKOUT_1_kHz);

  // check the oscillator stop flag on the RTC and give it a new time if necessary
  if (rtc.hasLostTime()) {
    rtc.setTime(tm, PCF2129AT_AM);
  }
  // else get the good time from the RTC
  else {
    rtc.getTime(tm);
  }

  // enable a falling edge interrupt on the square wave pin
  EICRA = (0x2 << (2*RTC_EXT_INT));
  EIMSK = (1 << RTC_EXT_INT);

  // set up the heartbeat led
  DDRB |= (1<<3);
  PORTB |= (1<<3);

  // set the display mode
  leds.setMode(DISPLAY_MODE_BLEND);

  // enable inputs
  buttons.init();

  // take care of unused pins
  initUnusedPins();

  // end setup - enable interrupts
  sei();

  // get lost
  for (;;) {
    
    // update the arms on a tick
    if (tick) {
      // clear the flag
      tick = false;
      // get the time
      if (++fr >= 32) {
        // beat the heart every second
        //beatHeart();
        fr = 0;
        rtc.getTime(tm);
      }
      // update the clock arms
      updateArms(tm[2], tm[1], tm[0], fr, dots);
    }

    uint8_t buttonState = 0;
    // take care of any switch presses
    if (buttons.getPress(&buttonState)) {
      beatHeart();
      if (buttonState & INPUT_MIN) {
        tm[1]++;
        if (tm[1] > 59) {
          tm[1] -= 60;
        }
        rtc.setTime(tm, PCF2129AT_AM);
      }
      if (buttonState & INPUT_HOUR) {
        tm[2]++;
        if (tm[2] > 12) {
          tm[2] -= 12;
        }
        rtc.setTime(tm, PCF2129AT_AM);
      }
      if (buttonState & INPUT_MODE) {
        uint8_t m = leds.getMode();
        m = (m < DISPLAY_NUM_MODES-1) ? m+1 : 0;
        leds.setMode(m);
      }
    }

    // take care of any switch holds
    if (buttons.getHold(&buttonState)) {
      beatHeart();
      //handleButtonHold(buttonState, tm);
    }
  }

  // one day we might get the answer
  return 42;
}


// update the clock arms
// dots array structure: { hr0, mn0, sc0, hr1, mn1, sc1, ... , hr11, mn11, sc11 }
void updateArms(uint8_t hour, uint8_t min, uint8_t sec, uint8_t frame, uint16_t *dots) {
  // get the display
  leds.getDisplay(hour, min, sec, frame, dots);
  // send to the LED driver
  tlc.setGS(dots);
}

// initialize unused pins as inputs with pullups enabled
void initUnusedPins(void) {
  // PORTB
  DDRB &= ~UNUSED_PORTB_MASK;
  PORTB |= UNUSED_PORTB_MASK;
  // PORTA
  DDRA &= ~UNUSED_PORTA_MASK;
  PORTA |= UNUSED_PORTA_MASK;
}

/*
// button handling logic
void handleButtonPress(uint8_t state, uint8_t *tm) {
  // if hour switch, increment the hours by 1
  if (state == INPUT_HOUR) {
    rtc.getTime(tm);
    if (++tm[2] > 12) {
      tm[2] -= 12;
    }
    rtc.setTime(tm, PCF2129AT_AM);
  }
  // if minute switch, increment minutes
  else if (state == INPUT_MIN) {
    rtc.getTime(tm);
    if (++tm[1] > 59) {
      tm[1] -= 60;
    }
    rtc.setTime(tm, PCF2129AT_AM);
  }
  // if mode switch, increment mode
  else if (state == INPUT_MODE) {
    uint8_t m = leds.getMode();
    m = (m < DISPLAY_NUM_MODES-1) ? m+1 : 0;
    leds.setMode(m);
  }
}

void handleButtonHold(uint8_t state, uint8_t *tm) {
  
  // if the hour switch is held, increment the hours by 3
  if (state == INPUT_HOUR) {
    tm[2] = (tm[2] + 3);
    if (tm[2] > 12) {
      tm[2] -= 12;
    } 
  }
  // else if the minute switch is held, increment the minutes by 5
  else if (state == INPUT_MIN) {
    tm[1] = (tm[1] + 5);
    if (tm[1] > 59) {
      tm[1] -= 60;
    }
  }
  
}
*/

void beatHeart() {
  PINB |= (1<<3);
}
