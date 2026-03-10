#pragma once

#ifdef __has_include
#if __has_include("ConfigUser.h")
#include "ConfigUser.h"
#endif
#endif

// WiFi
#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD ""
#endif

// MQTT Broker
#ifndef MQTT_HOST
#define MQTT_HOST "192.168.0.50"
#endif

#ifndef MQTT_PORT
#define MQTT_PORT 1883
#endif

#ifndef MQTT_USER
#define MQTT_USER ""
#endif

#ifndef MQTT_PASS
#define MQTT_PASS ""
#endif

// MQTT Topics - format EM24 pour Victron (dbus-mqtt-grid)
#ifndef MQTT_TOPIC_PREFIX
#define MQTT_TOPIC_PREFIX "linky"
#endif

// Intervalle de publication MQTT (ms)
#ifndef MQTT_PUBLISH_INTERVAL_MS
#define MQTT_PUBLISH_INTERVAL_MS 2000
#endif
