#pragma once

#ifdef DEVICE_QTBITS

#include <Arduino.h>
#include <Inputs/InputKeys.h>
#include <Interfaces/IDeviceView.h>

#define NVS_SSID_KEY "ssid"
#define NVS_PASS_KEY "pass"

#define DARK_GREY 0x4208

bool setupQtbitsWifi(IDeviceView& view);

#endif
