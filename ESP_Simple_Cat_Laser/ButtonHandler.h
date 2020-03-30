#pragma once
#ifndef BUTTONHANDLER_H_
#define BUTTONHANDLER_H_

#include <Arduino.h>
#include <functional>
#include <FunctionalInterrupt.h>

enum button_event {
  BUTTON_EVENT_SINGLECLICK = 0,
  BUTTON_EVENT_DOUBLECLICK,
  BUTTON_EVENT_LONGCLICK
};

class ButtonHandler {

  private:
    std::function<void(const button_event)> callback;
    std::function<bool(void)> is_down_function;

    bool longclicked = false;
    bool down_handled = false;

    bool wait_doubleclick = false;
    uint32_t down_time = 0;
    uint32_t up_time = 0;

    uint32_t longclick_threshold = 5000;
    uint32_t doubleclick_threshold = 200;

    bool longclick_enable = true;
    bool doubleclick_enable = true;

  public:
    ButtonHandler(uint32_t longclick_duration = 5000, uint32_t doubleclick_duration = 200) :
      longclick_threshold(longclick_duration),
      doubleclick_threshold(doubleclick_duration) {
    }

    void setCallback(std::function<void(const button_event)> callback) {
      this->callback = callback;
    }

    void setIsDownFunction(std::function<bool(void)> is_down_function) {
      this->is_down_function = is_down_function;
    }

    void setLongClickEnable(bool enable) {
      longclick_enable = enable;
    }
    void setDoubleClickEnable(bool enable) {
      doubleclick_enable = enable;
    }

    void handleChange(bool down) {

      if (down) {
        if (wait_doubleclick && doubleclick_enable) {
          down_handled = true;
          if (callback) {
            callback(BUTTON_EVENT_DOUBLECLICK);
          }
        } else {
          down_handled = false;
        }
        down_time = millis();
        longclicked = false;
        wait_doubleclick = false;
      } else { //up
        if (!down_handled) {
          if (doubleclick_enable) {
            up_time = millis();
            wait_doubleclick = true;
          } else {
            down_handled = true;
            if (callback) {
              callback(BUTTON_EVENT_SINGLECLICK);
            }
          }
        }
      }
    }

    void loop() {
      bool down = is_down_function();
      if (down) {
        if (longclick_enable) {
          if (!longclicked && !down_handled) {
            if (millis() - down_time > longclick_threshold) {
              //key2DoLongClick();
              longclicked = true;
              down_handled = true;
              if (callback) {
                callback(BUTTON_EVENT_LONGCLICK);
              }
            }
          }
        }
      } else { //up
        longclicked = false;
        if (wait_doubleclick && doubleclick_enable) {
          if (millis() - up_time > doubleclick_threshold) {
            wait_doubleclick = false;
            down_handled = true;
            //key2DoClick();
            if (callback) {
              callback(BUTTON_EVENT_SINGLECLICK);
            }
          }
        }

      }
    }
};

#endif /* BUTTONHANDLER_H_ */
