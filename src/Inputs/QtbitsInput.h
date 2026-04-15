#pragma once

#ifdef DEVICE_QTBITS

#include "Interfaces/IInput.h"
#include <RotaryEncoder.h>
#include <Arduino.h>

#ifndef PIN_ENC_A
  #define PIN_ENC_A 41
#endif
#ifndef PIN_ENC_B
  #define PIN_ENC_B 42
#endif
#ifndef PIN_ENC_BTN
  #define PIN_ENC_BTN 0
#endif
#ifndef PIN_SIDE_BTN
  #define PIN_SIDE_BTN 0
#endif

class QtbitsInput : public IInput {
public:
  QtbitsInput();

  char handler() override;
  char readChar() override;
  void waitPress(uint32_t timeoutMs) override;

  void tick();
  void checkShutdownRequest();
  void shutdownToDeepSleep();

private:
  RotaryEncoder encoder;
  char lastInput;
  char lastButton;
  int lastPos;
  unsigned long pressStart;
};

#endif
