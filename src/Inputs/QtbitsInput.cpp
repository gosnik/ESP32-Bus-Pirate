#ifdef DEVICE_QTBITS

#include "QtbitsInput.h"
#include "Inputs/InputKeys.h"
#include <esp_sleep.h>

QtbitsInput::QtbitsInput()
    : encoder(PIN_ENC_A, PIN_ENC_B, RotaryEncoder::LatchMode::TWO03),
      lastInput(KEY_NONE),
      lastPos(0),
      lastButton(false),
      pressStart(0) {
  encoder.setPosition(0);
  pinMode(PIN_ENC_BTN, INPUT_PULLUP);
  pinMode(PIN_SIDE_BTN, INPUT_PULLUP);
}

void QtbitsInput::tick() {
  encoder.tick();

  int pos = encoder.getPosition();
  if (pos < lastPos) {
    lastInput = KEY_ARROW_LEFT;
    lastPos = pos;
  } else if (pos > lastPos) {
    lastInput = KEY_ARROW_RIGHT;
    lastPos = pos;
  } else if (!digitalRead(PIN_ENC_BTN) && !lastButton) {
    lastInput = KEY_OK;
    lastButton = true;
  } else if (digitalRead(PIN_ENC_BTN)) {
    lastButton = false;
  }

  checkShutdownRequest();
}

char QtbitsInput::readChar() {
  tick();
  char c = lastInput;
  lastInput = KEY_NONE;
  return c;
}

char QtbitsInput::handler() {
  while (true) {
    char c = readChar();
    if (c != KEY_NONE) return c;
    delay(5);
  }
}

void QtbitsInput::waitPress(uint32_t timeoutMs) {
  uint32_t start = millis();
  while (true) {
    if (readChar() != KEY_NONE) return;
    if (timeoutMs > 0 && (millis() - start) >= timeoutMs) return;
    delay(5);
  }
}

void QtbitsInput::checkShutdownRequest() {
  if (!digitalRead(PIN_ENC_BTN) || !digitalRead(PIN_SIDE_BTN)) {
    for (int i = 2; i > 0; --i) {
      for (int j = 0; j < 10; ++j) {
        if (digitalRead(PIN_ENC_BTN) && digitalRead(PIN_SIDE_BTN)) return;
        delay(100);
      }
    }
    shutdownToDeepSleep();
  }
}

void QtbitsInput::shutdownToDeepSleep() {
  delay(2000);
  esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_SIDE_BTN, 0);
  esp_deep_sleep_start();
}

#endif
