#pragma once

#include <LibTeleinfo.h>
#include <SoftwareSerial.h>

#include "TeleinfoData.h"

class TeleinfoReader {
 public:
  static constexpr uint32_t kModeSwitchTimeoutMs = 8000;

  TeleinfoReader(uint8_t rxPin);

  void begin();
  void update();

  const TeleinfoData& data() const { return data_; }
  TeleinfoProtocol protocol() const { return protocol_; }
  long baudrate() const { return kModes[modeIndex_].baudrate; }
  const char* modeLabel() const { return kModes[modeIndex_].label; }
  const char* protocolLabel() const;
  bool autoBaud() const { return autoBaud_; }

 private:
  struct ModeConfig {
    long baudrate;
    _Mode_e mode;
    const char* label;
  };

  static constexpr ModeConfig kModes[] = {
      {9600, TINFO_MODE_STANDARD, "STD"},
      {1200, TINFO_MODE_HISTORIQUE, "HIS"},
  };
  static constexpr size_t kModeCount = sizeof(kModes) / sizeof(kModes[0]);

  static int roundTo50(int value);
  static const char* findLabelValue(ValueList* root, const char* label);
  static bool readLabelInt(ValueList* root, const char* label, int& out);
  static bool readLabelInt64(ValueList* root, const char* label, int64_t& out);

  void beginMode(size_t modeIndex);
  void maybeSwitchMode();
  void parseFrame(ValueList* list);
  void recomputeDerived(TeleinfoData& d);
  void updateProtocol(ValueList* list);

  static void onNewFrame(ValueList* list);
  static void onUpdatedFrame(ValueList* list);

  uint8_t rxPin_;
  bool autoBaud_ = true;
  SoftwareSerial serial_;
  TInfo tinfo_;
  TeleinfoData data_;
  TeleinfoProtocol protocol_ = TeleinfoProtocol::Unknown;
  size_t modeIndex_ = 0;
  uint32_t modeStartedMs_ = 0;
  uint32_t lastValidFrameMs_ = 0;

  static TeleinfoReader* instance_;
};
