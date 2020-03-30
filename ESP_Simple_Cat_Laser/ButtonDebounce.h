#pragma once
#ifndef BUTTON_DEBOUNCE_H_
#define BUTTON_DEBOUNCE_H_

#include <Arduino.h>
#include <functional>
#include <FunctionalInterrupt.h>

class ButtonDebounce {

  private:

    std::function<void(const bool)> callback;
    uint32_t lastchange_ms = 0;
    uint32_t debounce_ms = 35;
    bool laststate_is_down = false; // true is down;
    int pin_down_digital = LOW;
    int pin;

    bool readIsDown() {
      return pin_down_digital == digitalRead(pin);
    }

  public:
    //IRAM_ATTR
    ButtonDebounce(uint8_t pin, uint8_t pin_mode, uint8_t pin_down_digital,
                   uint32_t debounce_ms = 35) :
      pin(pin), pin_down_digital(pin_down_digital), debounce_ms(debounce_ms) {

      pinMode(pin, pin_mode);

    }

    void update(bool down) {
      const uint32_t t = millis();
      if (t - lastchange_ms < debounce_ms) {
        lastchange_ms = t;

      } else {
        if (laststate_is_down == down) {
        } else {
          lastchange_ms = t;
          laststate_is_down = down;
          if (callback) {
            callback(down);
          }
        }
      }
    }

    void update() {
      bool down = readIsDown();
      update(down);
    }

    bool checkIsDown() {
      return laststate_is_down;
    }

    void setCallback(std::function<void(const bool down)> callback) {
      this->callback = callback;
    }

    void setInterrupt(std::function<void(void)> interrupt_function) {
      if (interrupt_function) {
        attachInterrupt(digitalPinToInterrupt(pin), interrupt_function, CHANGE);
      }
    }

};

#endif
