#include "MqttPublisher.h"

#include <Arduino.h>
#include <stdio.h>
#include <string.h>

MqttPublisher::MqttPublisher(const char* fallbackHost, uint16_t fallbackPort,
                             const char* user, const char* pass,
                             const char* topicPrefix, uint32_t publishIntervalMs)
    : fallbackHost_(fallbackHost),
      fallbackPort_(fallbackPort),
      user_(user),
      pass_(pass),
      publishIntervalMs_(publishIntervalMs) {
  strncpy(topicPrefix_, topicPrefix, sizeof(topicPrefix_) - 1);
  topicPrefix_[sizeof(topicPrefix_) - 1] = '\0';
}

void MqttPublisher::begin() {
  mqtt_.setClient(wifiClient_);
  snprintf(clientId_, sizeof(clientId_), "linky-em24-%06lX",
           static_cast<unsigned long>(ESP.getChipId()));
}

bool MqttPublisher::discoverBroker() {
  const uint32_t now = millis();
  if (now - lastDiscoveryMs_ < kDiscoveryIntervalMs) return brokerDiscovered_;
  lastDiscoveryMs_ = now;

  if (!mdnsStarted_) {
    if (!MDNS.begin(clientId_)) return false;
    mdnsStarted_ = true;
  }

  MDNS.update();

  int n = MDNS.queryService("mqtt", "tcp");
  if (n > 0) {
    brokerIP_ = MDNS.IP(0);
    brokerPort_ = MDNS.port(0);
    brokerDiscovered_ = true;
    mqtt_.setServer(brokerIP_, brokerPort_);
    return true;
  }

  // Fallback sur l'hote configure (IP ou DNS)
  if (!brokerDiscovered_) {
    brokerPort_ = fallbackPort_;
    if (brokerIP_.fromString(fallbackHost_) || WiFi.hostByName(fallbackHost_, brokerIP_)) {
      mqtt_.setServer(brokerIP_, brokerPort_);
    } else {
      brokerIP_ = IPAddress(0, 0, 0, 0);
      mqtt_.setServer(fallbackHost_, brokerPort_);
    }
    brokerDiscovered_ = true;
  }

  return brokerDiscovered_;
}

void MqttPublisher::update(bool wifiConnected, const TeleinfoData& data) {
  if (!wifiConnected) return;

  if (!brokerDiscovered_) {
    discoverBroker();
    if (!brokerDiscovered_) return;
  }

  if (!mqtt_.connected()) {
    const uint32_t now = millis();
    if (now - lastReconnectMs_ < kReconnectIntervalMs) return;
    lastReconnectMs_ = now;

    // Re-tenter la decouverte si connexion echoue
    discoverBroker();
    reconnect();
    if (!mqtt_.connected()) return;
  }

  mqtt_.loop();

  if (!data.valid) return;

  const uint32_t now = millis();
  if (now - lastPublishMs_ < publishIntervalMs_) return;
  lastPublishMs_ = now;

  publishGrid(data);
}

void MqttPublisher::reconnect() {
  if (user_[0] != '\0') {
    mqtt_.connect(clientId_, user_, pass_);
  } else {
    mqtt_.connect(clientId_);
  }
}

void MqttPublisher::publish(const char* suffix, const char* value) {
  char topic[128];
  snprintf(topic, sizeof(topic), "%s/%s", topicPrefix_, suffix);
  mqtt_.publish(topic, value, true);
}

void MqttPublisher::publishInt(const char* suffix, int value) {
  char buf[16];
  snprintf(buf, sizeof(buf), "%d", value);
  publish(suffix, buf);
}

void MqttPublisher::publishFloat(const char* suffix, float value) {
  char buf[16];
  dtostrf(value, 1, 1, buf);
  publish(suffix, buf);
}

void MqttPublisher::publishGrid(const TeleinfoData& data) {
  publishInt("Ac/Power", data.totalPowerInst);

  const char* phaseNames[] = {"Ac/L1", "Ac/L2", "Ac/L3"};
  for (size_t i = 0; i < 3; ++i) {
    char suffix[32];

    snprintf(suffix, sizeof(suffix), "%s/Voltage", phaseNames[i]);
    publishInt(suffix, data.voltage[i]);

    snprintf(suffix, sizeof(suffix), "%s/Current", phaseNames[i]);
    publishInt(suffix, data.current[i]);

    snprintf(suffix, sizeof(suffix), "%s/Power", phaseNames[i]);
    publishInt(suffix, data.phasePowerInst[i]);
  }

  const float energyForward = static_cast<float>(data.indexHC + data.indexHP) / 10.0f;
  const float energyReverse = static_cast<float>(data.indexInj) / 10.0f;

  publishFloat("Ac/Energy/Forward", energyForward);
  publishFloat("Ac/Energy/Reverse", energyReverse);
}
