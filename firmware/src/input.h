// clockblock
// Copyright 2013 by Wiley Cousins, LLC.
// shared under the terms of the MIT License
//
// file: input.h
// description: header file for the clockblock user input class

#ifndef CLOCKBLOCK_INPUT_H
#define CLOCKBLOCK_INPUT_H

// typedefs
#include <stdint.h>

#define INPUT_DEBOUNCE_COUNT  1
#define INPUT_HOLD_COUNT      15

class Input {
public:
  // contructor - gives private variables default values
  Input(void);

  // get the flag states and clear as necessary
  bool getHold(uint8_t *s);
  bool getPress(uint8_t *s);
  // handle pin change
  void handleChange(void);
  // handle debouncing
  void handleTimer(void);

  // disable the debounce timer and wait for a pin change (e.g. to exit a pin hold loop)
  // void reset(void);
  // initialization
  void init(void);

private:
  // get switch state
  uint8_t getState(void);

  // init switch pins as inputs with pullups enabled
  void initPins(void);
  // init pin change interrupts
  void initInt(void);
  // init timer for debouncing
  void initTimer(void);

  // interrupt helpers
  void enableInt(void);
  void disableInt(void);
  void enableTimer(void);
  void disableTimer(void);

  // ISR volatiles
  // switch states
  volatile uint8_t state;
  volatile uint8_t trigState;
  volatile uint8_t pressState;
  volatile uint8_t holdState;
  // switch timer counter
  volatile uint8_t timerCount;
  // switch event flags
  volatile bool release;
  volatile bool press;
  volatile bool hold;
};

#endif
