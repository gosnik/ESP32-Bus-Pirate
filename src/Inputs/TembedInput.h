#pragma once

#if defined(DEVICE_TEMBEDS3) || defined(DEVICE_TEMBEDS3CC1101)

#include "Interfaces/IInput.h"
#include <RotaryEncoder.h>
#include <Arduino.h>
// #include <Views/TembedDeviceView.h>

#ifndef TEMBED_PIN_ENCODE_A
    #ifdef PIN_ENC_A
        #define TEMBED_PIN_ENCODE_A PIN_ENC_A
    #elif defined(DEVICE_TEMBEDS3CC1101)
        #define TEMBED_PIN_ENCODE_A 4
    #else
        #define TEMBED_PIN_ENCODE_A 2
    #endif
#endif

#ifndef TEMBED_PIN_ENCODE_B
    #ifdef PIN_ENC_B
        #define TEMBED_PIN_ENCODE_B PIN_ENC_B
    #elif defined(DEVICE_TEMBEDS3CC1101)
        #define TEMBED_PIN_ENCODE_B 5
    #else
        #define TEMBED_PIN_ENCODE_B 1
    #endif
#endif

#ifndef TEMBED_PIN_SIDE_BTN
    #ifdef PIN_SIDE_BTN
        #define TEMBED_PIN_SIDE_BTN PIN_SIDE_BTN
    #elif defined(DEVICE_TEMBEDS3CC1101)
        #define TEMBED_PIN_SIDE_BTN 6
    #else
        #define TEMBED_PIN_SIDE_BTN 0
    #endif
#endif

#ifndef TEMBED_PIN_ENCODE_BTN
    #ifdef PIN_ENC_BTN
        #define TEMBED_PIN_ENCODE_BTN PIN_ENC_BTN
    #else
        #define TEMBED_PIN_ENCODE_BTN 0
    #endif
#endif

class TembedInput : public IInput {
public:
    TembedInput();

    char handler() override;
    char readChar() override;
    void waitPress(uint32_t timeoutMs) override;

    void tick();
    void checkShutdownRequest();
    void shutdownToDeepSleep();

private:
    RotaryEncoder encoder;
    // TembedDeviceView view;
    char lastInput;
    char lastButton;
    int lastPos;
    unsigned long pressStart;
};

#endif
