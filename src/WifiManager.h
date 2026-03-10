#pragma once

#include <ESP8266WiFi.h>

class WifiManager {
 public:
  WifiManager(const char* ssid, const char* password);

  void begin();
  void update();

  bool isConnected() const { return WiFi.status() == WL_CONNECTED; }
  IPAddress localIP() const { return WiFi.localIP(); }
  int32_t rssi() const { return WiFi.RSSI(); }

 private:
  void connect();

  const char* ssid_;
  const char* password_;
  uint32_t lastAttemptMs_ = 0;
  static constexpr uint32_t kReconnectIntervalMs = 10000;
};
