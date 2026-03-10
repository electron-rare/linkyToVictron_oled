#pragma once

#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>

#include "TeleinfoData.h"

struct ConnStatus {
  bool wifiConnected;
  bool mqttConnected;
  int32_t rssi;
  IPAddress brokerIP;
};

class OledDisplay {
 public:
  static constexpr uint8_t kWidth = 128;
  static constexpr uint8_t kHeight = 64;
  static constexpr uint8_t kI2cAddress = 0x3C;
  static constexpr uint32_t kRefreshMs = 500;
  static constexpr uint32_t kPageSwitchMs = 4000;
  static constexpr uint8_t kPageCount = 3;

  OledDisplay(uint8_t sda, uint8_t scl);

  bool begin();
  void showInit();
  void update(const TeleinfoData& data, TeleinfoProtocol protocol,
              long baudrate, const char* modeLabel, bool autoBaud,
              const ConnStatus& conn);

 private:
  void drawWaiting(long baudrate, const char* modeLabel, bool autoBaud,
                   const ConnStatus& conn);
  void drawRealtime(const TeleinfoData& data, TeleinfoProtocol protocol,
                    long baudrate, const ConnStatus& conn);
  void drawEnergy(const TeleinfoData& data);
  void drawStatusBar(const ConnStatus& conn);

  static void formatTenthKwh(int64_t value, char* out, size_t outSize);
  static const char* protocolStr(TeleinfoProtocol p);

  uint8_t sda_;
  uint8_t scl_;
  Adafruit_SSD1306 display_;
  uint32_t lastRefreshMs_ = 0;
  uint32_t lastPageSwitchMs_ = 0;
  uint8_t currentPage_ = 0;
};
