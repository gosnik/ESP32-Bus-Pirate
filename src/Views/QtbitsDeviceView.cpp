#ifdef DEVICE_QTBITS

#include "QtbitsDeviceView.h"
#include "Data/WelcomeScreen.h"

QtbitsDeviceView::QtbitsDeviceView() {
  pinMode(TFT_EN, OUTPUT);
  digitalWrite(TFT_EN, HIGH);

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
}

SPIClass& QtbitsDeviceView::getSharedSpiInstance() {
  return sharedSpi;
}

void* QtbitsDeviceView::getScreen() {
  return &tft;
}

void QtbitsDeviceView::initialize() {
  pinMode(TFT_EN, OUTPUT);
  digitalWrite(TFT_EN, HIGH);

  screenSpi.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);

  tft.init();
  tft.setRotation(TFT_ROT);
  tft.setSwapBytes(true);

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  setBrightness(brightnessPct);

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
}

void QtbitsDeviceView::logo() {
  clear();

  tft.setSwapBytes(true);
  tft.pushImage(40, 30, WELCOME_IMAGE_WIDTH, WELCOME_IMAGE_HEIGHT, WelcomeScreen);
  tft.setSwapBytes(false);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  GlobalState& state = GlobalState::getInstance();
  auto version = "ESP32 Bus Pirate - " + state.getVersion();
  drawCenterText(version.c_str(), 130, 2);
}

void QtbitsDeviceView::welcome(TerminalTypeEnum& terminalType, std::string& terminalInfos) {
  if (terminalType == TerminalTypeEnum::WiFiClient) welcomeWeb(terminalInfos);
  else welcomeSerial(terminalInfos);
}

void QtbitsDeviceView::loading() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextFont(1);

  tft.fillRoundRect(20, 20, tft.width() - 40, tft.height() - 40, 5, QTBITS_DARK_GREY_RECT);
  tft.drawRoundRect(20, 20, tft.width() - 40, tft.height() - 40, 5, TFT_GREEN);

  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.drawString("Loading...", 102, 52);
}

void QtbitsDeviceView::clear() {
  tft.fillScreen(TFT_BLACK);
}

void QtbitsDeviceView::drawLogicTrace(uint8_t pin, const std::vector<uint8_t>& buffer, uint8_t step) {
  tft.fillRect(0, 35, tft.width(), tft.height() - 35, TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(1);
  tft.setTextSize(1);
  tft.setCursor(10, 10);
  tft.print("GPIO ");
  tft.print(pin);

  const int traceY = 50;
  const int traceH = 80;
  const int centerY = traceY + traceH / 2;

  int x = 10;
  for (size_t i = 1; i < buffer.size(); ++i) {
    uint8_t prev = buffer[i - 1];
    uint8_t curr = buffer[i];

    int y1 = prev ? (centerY - 15) : (centerY + 15);
    int y2 = curr ? (centerY - 15) : (centerY + 15);

    if (curr != prev) {
      tft.drawLine(x, y1, x + step, y1, prev ? TFT_GREEN : TFT_WHITE);
      tft.drawLine(x + step, y1, x + step, y2, curr ? TFT_GREEN : TFT_WHITE);
    } else {
      tft.drawLine(x, y1, x + step, y2, curr ? TFT_GREEN : TFT_WHITE);
    }

    x += step;
    if (x > tft.width() - step) break;
  }
}

void QtbitsDeviceView::drawAnalogicTrace(uint8_t pin, const std::vector<uint8_t>& buffer, uint8_t step) {
  tft.fillRect(0, 35, tft.width(), tft.height() - 35, TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(1);
  tft.setTextSize(1);
  tft.setCursor(10, 10);
  tft.print("GPIO ");
  tft.print(pin);

  const int topY = 35;
  const int h = 135;

  int x = 10;
  for (size_t i = 1; i < buffer.size(); ++i) {
    int prev = topY + (h - 1) - (buffer[i - 1] >> 1);
    int curr = topY + (h - 1) - (buffer[i] >> 1);
    tft.drawLine(x, prev, x + step, curr, TFT_GREEN);
    x += step;
    if (x > tft.width() - step) break;
  }
}

void QtbitsDeviceView::drawWaterfall(
    const std::string& title,
    float startValue,
    float endValue,
    const char* unit,
    int rowIndex,
    int rowCount,
    int level
) {
  const int W = tft.width();
  const int H = tft.height();
  const int midX = W / 2;

  const int headerH = 12;
  const int footerH = 12;
  const int graphY  = headerH;
  const int graphH  = H - headerH - footerH;
  const int barMaxPixels = midX - 2;

  if (level < 0) level = 0;
  if (level > 100) level = 100;
  int barPixels = (level * barMaxPixels) / 100;

  if (rowIndex == 0) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(1);
    tft.setTextFont(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(2, 2);
    tft.print(title.c_str());

    char bufStart[24];
    char bufEnd[24];
    if (unit && unit[0]) {
      snprintf(bufStart, sizeof(bufStart), "%.2f%s", startValue, unit);
      snprintf(bufEnd, sizeof(bufEnd), "%.2f%s", endValue, unit);
    } else {
      snprintf(bufStart, sizeof(bufStart), "%.2f", startValue);
      snprintf(bufEnd, sizeof(bufEnd), "%.2f", endValue);
    }

    int wStart = tft.textWidth(bufStart);
    tft.setCursor(W - wStart - 2, 2);
    tft.print(bufStart);

    int wEnd = tft.textWidth(bufEnd);
    tft.setCursor(W - wEnd - 2, H - footerH + 2);
    tft.print(bufEnd);

    tft.fillRect(0, graphY, W, graphH, TFT_BLACK);
    tft.drawFastVLine(midX, graphY, graphH, TFT_DARKGREY);
  }

  if (rowCount <= 1) return;
  if (rowIndex < 0) rowIndex = 0;
  if (rowIndex > rowCount - 1) rowIndex = rowCount - 1;

  int y = graphY + (int)((int64_t)rowIndex * (graphH - 1) / (rowCount - 1));
  tft.drawFastHLine(0, y, W, TFT_BLACK);
  tft.drawPixel(midX, y, TFT_DARKGREY);

  if (barPixels > 0) {
    int x0 = midX - barPixels;
    int w = barPixels * 2;
    if (w > 0) {
      tft.drawFastHLine(x0, y, w, TFT_GREEN);
    }
  }
}

void QtbitsDeviceView::setRotation(uint8_t rotation) {
  tft.setRotation(rotation);
}

void QtbitsDeviceView::setBrightness(uint8_t brightness) {
  brightnessPct = brightness;
  uint8_t pwm = map(brightnessPct, 0, 100, 0, 255);
  analogWrite(TFT_BL, pwm);
}

uint8_t QtbitsDeviceView::getBrightness() {
  return brightnessPct;
}

void QtbitsDeviceView::topBar(const std::string& title, bool submenu, bool searchBar) {
  tft.fillRect(0, 0, tft.width(), 30, TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(5, 8);

  if (submenu) tft.print("< ");
  tft.print(title.c_str());

  if (searchBar) {
    tft.drawRoundRect(tft.width() - 60, 6, 50, 18, 4, TFT_DARKGREY);
    tft.setTextSize(1);
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.setCursor(tft.width() - 50, 11);
    tft.print("Search");
  }
}

void QtbitsDeviceView::horizontalSelection(
  const std::vector<std::string>& options,
  uint16_t selectedIndex,
  const std::string& description1,
  const std::string& description2
) {
  clear();

  int boxY = 50;
  int boxH = 48;
  int gap = 10;
  int corner = 8;

  for (size_t i = 0; i < options.size(); ++i) {
    int boxW = tft.textWidth(options[i].c_str()) + 30;
    int totalW = boxW;
    int boxX = (tft.width() - totalW) / 2;
    int y = boxY + i * (boxH + gap);

    uint16_t bg = (i == selectedIndex) ? QTBITS_DARK_GREY_RECT : TFT_BLACK;
    uint16_t border = (i == selectedIndex) ? TFT_GREEN : TFT_DARKGREY;
    tft.fillRoundRect(boxX, y, boxW, boxH, corner, bg);
    tft.drawRoundRect(boxX, y, boxW, boxH, corner, border);

    tft.setTextColor(TFT_WHITE, bg);
    tft.setTextSize(2);
    int tx = boxX + (boxW - tft.textWidth(options[i].c_str())) / 2;
    int ty = y + (boxH - 16) / 2;
    tft.drawString(options[i].c_str(), tx, ty);
  }

  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  drawCenterText(description1, 15, 1);
  tft.setTextColor(TFT_WHITE, QTBITS_DARK_GREY_RECT);
  drawCenterText(description2, tft.height() - 18, 1);
}

void QtbitsDeviceView::drawCenterText(const std::string& text, int y, int fontSize) {
  tft.setTextSize(fontSize);
  int16_t x = (tft.width() - tft.textWidth(text.c_str())) / 2;
  tft.drawString(text.c_str(), x, y);
}

void QtbitsDeviceView::welcomeSerial(const std::string& baudStr) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);

  drawCenterText("Serial mode", 60, 2);

  tft.drawRoundRect(70, 60, 180, 40, 8, TFT_GREEN);
  tft.setTextSize(1);
  drawCenterText("USB CDC ready", 72, 1);

  tft.setTextColor(TFT_WHITE, QTBITS_DARK_GREY_RECT);
  tft.fillRoundRect(55, 110, 210, 28, 6, QTBITS_DARK_GREY_RECT);
  drawCenterText("Baud: " + baudStr, 118, 1);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  drawCenterText("Open a serial terminal to continue", 160, 1);
}

void QtbitsDeviceView::welcomeWeb(const std::string& ipStr) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);

  drawCenterText("Wi-Fi mode", 60, 2);

  tft.drawRoundRect(60, 60, 200, 40, 8, TFT_GREEN);
  tft.setTextSize(1);
  drawCenterText("Open in browser", 72, 1);

  tft.setTextColor(TFT_WHITE, QTBITS_DARK_GREY_RECT);
  tft.fillRoundRect(40, 110, 240, 28, 6, QTBITS_DARK_GREY_RECT);
  drawCenterText(ipStr, 118, 1);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  drawCenterText("Browse to the device IP", 160, 1);
}

void QtbitsDeviceView::shutDown() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  drawCenterText("Shutting down...", 80, 2);
  digitalWrite(TFT_BL, LOW);
  digitalWrite(TFT_EN, LOW);
}

void QtbitsDeviceView::show(PinoutConfig& config) {
  tft.fillScreen(TFT_BLACK);
  const auto& mappings = config.getMappings();
  auto mode = config.getMode();

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextDatum(MC_DATUM);
  std::string modeStr = "MODE " + mode;
  tft.drawString(modeStr.c_str(), tft.width() / 2, 20);
  tft.setTextDatum(TL_DATUM);

  if (mappings.empty()) {
    const int frameX = 20;
    const int frameY = 45;
    const int frameW = tft.width() - 40;
    const int frameH = tft.height() - 70;
    const int frameR = 5;

    tft.fillRoundRect(frameX, frameY, frameW, frameH, frameR, TFT_BLACK);
    tft.drawRoundRect(frameX, frameY, frameW, frameH, frameR, TFT_GREEN);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextFont(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Nothing to display", tft.width() / 2, frameY + frameH / 2);
    tft.setTextDatum(TL_DATUM);
    return;
  }

  int boxHeight = 24;
  int startY = 40;
  for (size_t i = 0; i < mappings.size(); ++i) {
    int y = startY + (int)i * (boxHeight + 4);
    tft.fillRoundRect(20, y, tft.width() - 40, boxHeight, 6, QTBITS_DARK_GREY_RECT);
    tft.drawRoundRect(20, y, tft.width() - 40, boxHeight, 6, TFT_GREEN);

    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, QTBITS_DARK_GREY_RECT);

    int w = tft.textWidth(mappings[i].c_str());
    int textX = (tft.width() - w) / 2;
    tft.setCursor(textX, y + 5);
    tft.print(mappings[i].c_str());
  }
}

#endif
