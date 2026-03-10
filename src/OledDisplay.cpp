#include "OledDisplay.h"

#include <Arduino.h>
#include <Wire.h>
#include <stdio.h>

OledDisplay::OledDisplay(uint8_t sda, uint8_t scl)
    : sda_(sda), scl_(scl), display_(kWidth, kHeight, &Wire, -1) {}

bool OledDisplay::begin() {
  Wire.begin(sda_, scl_);
  Wire.setClock(400000);
  return display_.begin(SSD1306_SWITCHCAPVCC, kI2cAddress);
}

void OledDisplay::showInit() {
  display_.clearDisplay();
  display_.setTextSize(1);
  display_.setTextColor(SSD1306_WHITE);
  display_.setCursor(0, 0);
  display_.println(F("Init Linky TRI"));
  display_.println(F("SDA:D1 SCL:D2"));
  display_.println(F("TIC RX:D7"));
  display_.println(F("LibTeleinfo+MQTT"));
  display_.display();
}

void OledDisplay::drawStatusBar(const ConnStatus& conn) {
  // Derniere ligne (y=56) : barre de statut WiFi/MQTT
  char bar[22];
  if (!conn.wifiConnected) {
    snprintf(bar, sizeof(bar), "WiFi:-- MQTT:--");
  } else if (!conn.mqttConnected) {
    snprintf(bar, sizeof(bar), "WiFi:%ddB MQTT:--", conn.rssi);
  } else {
    snprintf(bar, sizeof(bar), "WiFi:%ddB MQTT:OK", conn.rssi);
  }
  display_.setCursor(0, 56);
  display_.print(bar);
}

void OledDisplay::update(const TeleinfoData& data, TeleinfoProtocol protocol,
                         long baudrate, const char* modeLabel, bool autoBaud,
                         const ConnStatus& conn) {
  const uint32_t now = millis();

  if (now - lastPageSwitchMs_ >= kPageSwitchMs) {
    lastPageSwitchMs_ = now;
    currentPage_ = (currentPage_ + 1) % kPageCount;
  }

  if (now - lastRefreshMs_ < kRefreshMs) return;
  lastRefreshMs_ = now;

  if (!data.valid) {
    drawWaiting(baudrate, modeLabel, autoBaud, conn);
    return;
  }

  switch (currentPage_) {
    case 0: drawRealtime(data, protocol, baudrate, conn); break;
    case 1: drawEnergy(data); break;
    case 2: // Page statut connexion
      display_.clearDisplay();
      display_.setTextSize(1);
      display_.setTextColor(SSD1306_WHITE);
      display_.setCursor(0, 0);
      display_.println(F("Linky TRI Network"));
      {
        char line[40];
        snprintf(line, sizeof(line), "WiFi: %s", conn.wifiConnected ? "OK" : "...");
        display_.println(line);
        if (conn.wifiConnected) {
          snprintf(line, sizeof(line), "IP:%s", WiFi.localIP().toString().c_str());
          display_.println(line);
          snprintf(line, sizeof(line), "RSSI: %d dBm", conn.rssi);
          display_.println(line);
        } else {
          display_.println(F("IP: --"));
          display_.println(F("RSSI: --"));
        }
        snprintf(line, sizeof(line), "MQTT: %s", conn.mqttConnected ? "OK" : "...");
        display_.println(line);
        if (conn.mqttConnected) {
          snprintf(line, sizeof(line), "Broker:%s", conn.brokerIP.toString().c_str());
          display_.println(line);
        }
      }
      display_.display();
      break;
  }
}

void OledDisplay::drawWaiting(long baudrate, const char* modeLabel,
                               bool autoBaud, const ConnStatus& conn) {
  display_.clearDisplay();
  display_.setTextSize(1);
  display_.setTextColor(SSD1306_WHITE);
  display_.setCursor(0, 0);
  display_.println(F("Linky TRI OLED"));
  display_.println(F("Attente trame..."));
  display_.print(F("Baud: "));
  display_.println(baudrate);
  display_.print(F("Mode: "));
  display_.println(modeLabel);
  display_.println(autoBaud ? F("Auto 9600/1200 ON") : F("Auto 9600/1200 OFF"));

  {
    char line[22];
    snprintf(line, sizeof(line), "WiFi:%s MQTT:%s",
             conn.wifiConnected ? "OK" : "--",
             conn.mqttConnected ? "OK" : "--");
    display_.println(line);
  }

  display_.display();
}

void OledDisplay::drawRealtime(const TeleinfoData& data,
                               TeleinfoProtocol protocol, long baudrate,
                               const ConnStatus& conn) {
  char line[40];

  display_.clearDisplay();
  display_.setTextSize(1);
  display_.setTextColor(SSD1306_WHITE);
  display_.setCursor(0, 0);

  snprintf(line, sizeof(line), "PT:%6dW T:%s", data.totalPowerInst, data.tarif);
  display_.println(line);

  for (size_t i = 0; i < 3; ++i) {
    snprintf(line, sizeof(line), "L%u U%3d I%3d P%5d",
             static_cast<unsigned int>(i + 1),
             data.voltage[i], data.current[i], data.phasePowerInst[i]);
    display_.println(line);
  }

  snprintf(line, sizeof(line), "B%ld %s", baudrate, protocolStr(protocol));
  display_.println(line);

  display_.println();
  drawStatusBar(conn);

  display_.display();
}

void OledDisplay::drawEnergy(const TeleinfoData& data) {
  char hcText[24], hpText[24], injText[24], totalText[24], line[40];

  formatTenthKwh(data.indexHC, hcText, sizeof(hcText));
  formatTenthKwh(data.indexHP, hpText, sizeof(hpText));
  formatTenthKwh(data.indexInj, injText, sizeof(injText));
  formatTenthKwh(data.indexHC + data.indexHP, totalText, sizeof(totalText));

  display_.clearDisplay();
  display_.setTextSize(1);
  display_.setTextColor(SSD1306_WHITE);
  display_.setCursor(0, 0);

  display_.println(F("Linky TRI Indexes"));

  snprintf(line, sizeof(line), "HC : %s kWh", hcText);
  display_.println(line);
  snprintf(line, sizeof(line), "HP : %s kWh", hpText);
  display_.println(line);
  snprintf(line, sizeof(line), "INJ: %s kWh", injText);
  display_.println(line);
  snprintf(line, sizeof(line), "TOT: %s kWh", totalText);
  display_.println(line);
  snprintf(line, sizeof(line), "Tarif: %s", data.tarif);
  display_.println(line);

  display_.display();
}

void OledDisplay::formatTenthKwh(int64_t value, char* out, size_t outSize) {
  const bool negative = value < 0;
  const uint64_t absVal = negative ? static_cast<uint64_t>(-value)
                                   : static_cast<uint64_t>(value);
  const unsigned long long whole = static_cast<unsigned long long>(absVal / 10ULL);
  const unsigned int decimal = static_cast<unsigned int>(absVal % 10ULL);
  snprintf(out, outSize, "%s%llu.%u", negative ? "-" : "", whole, decimal);
}

const char* OledDisplay::protocolStr(TeleinfoProtocol p) {
  switch (p) {
    case TeleinfoProtocol::Standard:  return "STD";
    case TeleinfoProtocol::Historical: return "HIS";
    default: return "UNK";
  }
}
