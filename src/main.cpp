#include <Arduino.h>

#include "Config.h"
#include "MqttPublisher.h"
#include "OledDisplay.h"
#include "TeleinfoReader.h"
#include "WifiManager.h"

namespace {
constexpr uint8_t PIN_SDA = D1;
constexpr uint8_t PIN_SCL = D2;
constexpr uint8_t PIN_TELEINFO_RX = D7;

OledDisplay oled(PIN_SDA, PIN_SCL);
TeleinfoReader teleinfo(PIN_TELEINFO_RX);
WifiManager wifi(WIFI_SSID, WIFI_PASSWORD);
MqttPublisher mqtt(MQTT_HOST, MQTT_PORT, MQTT_USER, MQTT_PASS,
                   MQTT_TOPIC_PREFIX, MQTT_PUBLISH_INTERVAL_MS);
}  // namespace

void setup() {
  Serial.begin(9600);
  Serial.println(F("\n[Linky TRI] Boot"));

  if (!oled.begin()) {
    Serial.println(F("[OLED] FAIL"));
    while (true) delay(1000);
  }
  oled.showInit();

  teleinfo.begin();
  Serial.println(F("[TIC] OK"));

  wifi.begin();
  Serial.println(F("[WiFi] Connecting..."));

  mqtt.begin();
  Serial.println(F("[MQTT] Ready"));
}

void loop() {
  teleinfo.update();
  wifi.update();
  mqtt.update(wifi.isConnected(), teleinfo.data());

  ConnStatus conn;
  conn.wifiConnected = wifi.isConnected();
  conn.mqttConnected = mqtt.isConnected();
  conn.rssi = wifi.rssi();
  conn.brokerIP = mqtt.brokerIP();

  oled.update(teleinfo.data(), teleinfo.protocol(),
              teleinfo.baudrate(), teleinfo.modeLabel(),
              teleinfo.autoBaud(), conn);
}
