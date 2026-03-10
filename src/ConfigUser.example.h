#pragma once

// Copiez ce fichier en src/ConfigUser.h (ignore par git) et adaptez les valeurs.

#define WIFI_SSID "MonWifi"
#define WIFI_PASSWORD "MonMotDePasse"

// IP ou hostname du broker MQTT.
#define MQTT_HOST "192.168.0.50"
#define MQTT_PORT 1883

// Laisser vide si pas d'authentification.
#define MQTT_USER ""
#define MQTT_PASS ""

#define MQTT_TOPIC_PREFIX "linky"
#define MQTT_PUBLISH_INTERVAL_MS 2000
