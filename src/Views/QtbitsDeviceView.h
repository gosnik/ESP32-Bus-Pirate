#pragma once

#ifdef DEVICE_QTBITS

#include "Interfaces/IDeviceView.h"
#include "States/GlobalState.h"

#include <Arduino.h>
#include <LovyanGFX.hpp>

#ifndef TFT_BL
  #define TFT_BL 33
#endif
#ifndef TFT_MISO
  #define TFT_MISO -1
#endif
#ifndef TFT_MOSI
  #define TFT_MOSI 34
#endif
#ifndef TFT_SCLK
  #define TFT_SCLK 37
#endif
#ifndef TFT_CS
  #define TFT_CS 36
#endif
#ifndef TFT_DC
  #define TFT_DC 38
#endif
#ifndef TFT_RST
  #define TFT_RST 35
#endif
#ifndef TFT_EN
  #define TFT_EN 5
#endif
#ifndef TFT_ROT
  #define TFT_ROT 3
#endif

#define QTBITS_DARK_GREY_RECT 0x4208

class LGFX_Qtbits : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel;
  lgfx::Bus_SPI _bus;
  lgfx::Light_PWM _light;

public:
  LGFX_Qtbits() {
    {
      auto cfg = _bus.config();
      cfg.spi_host = SPI2_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;
      cfg.freq_read = 20000000;
      cfg.pin_sclk = TFT_SCLK;
      cfg.pin_mosi = TFT_MOSI;
      cfg.pin_miso = TFT_MISO;
      cfg.pin_dc = TFT_DC;
      cfg.spi_3wire = false;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      _bus.config(cfg);
      _panel.setBus(&_bus);
    }

    {
      auto cfg = _panel.config();
      cfg.pin_cs = TFT_CS;
      cfg.pin_rst = TFT_RST;
      cfg.pin_busy = -1;
      cfg.panel_width = 170;
      cfg.panel_height = 320;
      cfg.memory_width = 240;
      cfg.memory_height = 320;
      cfg.offset_x = (TFT_ROT == 1 || TFT_ROT == 3) ? 0 : 35;
      cfg.offset_y = (TFT_ROT == 1 || TFT_ROT == 3) ? 35 : 0;
      cfg.invert = true;
      cfg.rgb_order = false;
      cfg.dlen_16bit = false;
      _panel.config(cfg);
    }

    setPanel(&_panel);
  }
};

class QtbitsDeviceView : public IDeviceView {
public:
  QtbitsDeviceView();

  void initialize() override;
  SPIClass& getSharedSpiInstance() override;
  void* getScreen() override;
  void logo() override;
  void welcome(TerminalTypeEnum& terminalType, std::string& terminalInfos) override;
  void show(PinoutConfig& config) override;
  void loading() override;
  void clear() override;
  void drawLogicTrace(uint8_t pin, const std::vector<uint8_t>& buffer, uint8_t step) override;
  void drawAnalogicTrace(uint8_t pin, const std::vector<uint8_t>& buffer, uint8_t step) override;
  void drawWaterfall(const std::string& title, float startValue, float endValue, const char* unit, int rowIndex, int rowCount, int level) override;
  void setRotation(uint8_t rotation) override;
  void setBrightness(uint8_t brightness) override;
  uint8_t getBrightness() override;
  void topBar(const std::string& title, bool submenu, bool searchBar) override;
  void horizontalSelection(
    const std::vector<std::string>& options,
    uint16_t selectedIndex,
    const std::string& description1,
    const std::string& description2
  ) override;

  void shutDown();

private:
  LGFX_Qtbits tft;
  uint8_t brightnessPct = 100;
  SPIClass screenSpi{HSPI};
  SPIClass sharedSpi{HSPI};

  void drawCenterText(const std::string& text, int y, int fontSize);
  void welcomeWeb(const std::string& ip);
  void welcomeSerial(const std::string& baud);
};

#endif
