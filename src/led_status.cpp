#include "led_status.h"
#include <Adafruit_NeoPixel.h>

namespace {
  Adafruit_NeoPixel strip(1, 38, NEO_RGB + NEO_KHZ800);
  uint8_t ledPin = 38;
}

namespace LedStatus {
  void begin(uint8_t pin, uint8_t brightness) {
    ledPin = pin;
    strip.setPin(ledPin);
    strip.updateType(NEO_RGB + NEO_KHZ800);
    strip.updateLength(1);
    strip.begin();
    strip.setBrightness(brightness);
    strip.clear();
    strip.show();
  }

  void setRGB(uint8_t r, uint8_t g, uint8_t b) {
    strip.setPixelColor(0, strip.Color(r, g, b));
    strip.show();
  }

  void set(State s) {
    switch (s) {
      case State::Booting: // Bleu
        setRGB(0, 100, 255);
        break;
      case State::Ok: // Vert
        setRGB(0, 255, 0);
        break;
      case State::Error: // Rouge
        setRGB(255, 0, 0);
        break;
    }
  }

  void ok() { set(State::Ok); }
  void error() { set(State::Error); }
  void booting() { set(State::Booting); }
}
