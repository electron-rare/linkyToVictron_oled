#include "WifiManager.h"

#include <Arduino.h>

WifiManager::WifiManager(const char* ssid, const char* password)
    : ssid_(ssid), password_(password) {}

void WifiManager::begin() {
  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);
  connect();
}

void WifiManager::update() {
  if (isConnected()) return;

  const uint32_t now = millis();
  if (now - lastAttemptMs_ < kReconnectIntervalMs) return;

  connect();
}

void WifiManager::connect() {
  lastAttemptMs_ = millis();
  WiFi.begin(ssid_, password_);
}
