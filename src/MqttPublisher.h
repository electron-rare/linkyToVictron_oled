#pragma once

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>

#include "TeleinfoData.h"

class MqttPublisher {
 public:
  MqttPublisher(const char* fallbackHost, uint16_t fallbackPort,
                const char* user, const char* pass,
                const char* topicPrefix, uint32_t publishIntervalMs);

  void begin();
  void update(bool wifiConnected, const TeleinfoData& data);

  bool isConnected() { return mqtt_.connected(); }
  bool brokerDiscovered() const { return brokerDiscovered_; }
  IPAddress brokerIP() const { return brokerIP_; }
  uint16_t brokerPort() const { return brokerPort_; }

 private:
  bool discoverBroker();
  void reconnect();
  void publish(const char* suffix, const char* value);
  void publishInt(const char* suffix, int value);
  void publishFloat(const char* suffix, float value);
  void publishGrid(const TeleinfoData& data);

  WiFiClient wifiClient_;
  PubSubClient mqtt_;

  const char* fallbackHost_;
  uint16_t fallbackPort_;
  const char* user_;
  const char* pass_;
  char topicPrefix_[64];
  char clientId_[32] = "linky-em24";
  uint32_t publishIntervalMs_;

  bool brokerDiscovered_ = false;
  bool mdnsStarted_ = false;
  IPAddress brokerIP_;
  uint16_t brokerPort_ = 1883;

  uint32_t lastPublishMs_ = 0;
  uint32_t lastReconnectMs_ = 0;
  uint32_t lastDiscoveryMs_ = 0;
  static constexpr uint32_t kReconnectIntervalMs = 5000;
  static constexpr uint32_t kDiscoveryIntervalMs = 15000;
};
