#include "TeleinfoReader.h"

#include <Arduino.h>
#include <stdlib.h>
#include <string.h>

TeleinfoReader* TeleinfoReader::instance_ = nullptr;

constexpr TeleinfoReader::ModeConfig TeleinfoReader::kModes[];

TeleinfoReader::TeleinfoReader(uint8_t rxPin) : rxPin_(rxPin) {
  instance_ = this;
}

void TeleinfoReader::begin() {
  tinfo_.attachNewFrame(onNewFrame);
  tinfo_.attachUpdatedFrame(onUpdatedFrame);
  beginMode(0);
}

void TeleinfoReader::update() {
  while (serial_.available() > 0) {
    tinfo_.process(static_cast<char>(serial_.read()));
  }
  maybeSwitchMode();
}

const char* TeleinfoReader::protocolLabel() const {
  switch (protocol_) {
    case TeleinfoProtocol::Standard:
      return "STD";
    case TeleinfoProtocol::Historical:
      return "HIS";
    default:
      return "UNK";
  }
}

void TeleinfoReader::beginMode(size_t modeIndex) {
  modeIndex_ = modeIndex % kModeCount;
  serial_.begin(kModes[modeIndex_].baudrate, SWSERIAL_7E1, rxPin_, -1, false);
  while (serial_.available() > 0) {
    serial_.read();
  }
  tinfo_.init(kModes[modeIndex_].mode);

  protocol_ = (kModes[modeIndex_].mode == TINFO_MODE_STANDARD)
                   ? TeleinfoProtocol::Standard
                   : TeleinfoProtocol::Historical;

  data_.valid = false;
  modeStartedMs_ = millis();
  lastValidFrameMs_ = modeStartedMs_;
}

void TeleinfoReader::maybeSwitchMode() {
  if (!autoBaud_) return;

  const uint32_t now = millis();
  if ((now - modeStartedMs_) < kModeSwitchTimeoutMs) return;
  if ((now - lastValidFrameMs_) < kModeSwitchTimeoutMs) return;

  beginMode((modeIndex_ + 1) % kModeCount);
}

int TeleinfoReader::roundTo50(int value) {
  if (value >= 0) return (value / 50) * 50;
  return -(((-value) / 50) * 50);
}

const char* TeleinfoReader::findLabelValue(ValueList* root, const char* label) {
  if (!root || !label) return nullptr;
  ValueList* node = root;
  while (node->next) {
    node = node->next;
    if (!node->name || !node->value) continue;
    if (strcmp(node->name, label) == 0) return node->value;
  }
  return nullptr;
}

bool TeleinfoReader::readLabelInt(ValueList* root, const char* label, int& out) {
  const char* v = findLabelValue(root, label);
  if (!v || *v == '\0') return false;
  out = atoi(v);
  return true;
}

bool TeleinfoReader::readLabelInt64(ValueList* root, const char* label, int64_t& out) {
  const char* v = findLabelValue(root, label);
  if (!v || *v == '\0') return false;
  out = atoll(v);
  return true;
}

void TeleinfoReader::updateProtocol(ValueList* list) {
  if (findLabelValue(list, "DATE")) {
    protocol_ = TeleinfoProtocol::Standard;
    return;
  }
  if (findLabelValue(list, "ADCO")) {
    protocol_ = TeleinfoProtocol::Historical;
  }
}

void TeleinfoReader::parseFrame(ValueList* list) {
  if (!list) return;

  TeleinfoData d = data_;
  d.valid = true;

  const char* tarif = findLabelValue(list, "PTEC");
  if (!tarif) tarif = findLabelValue(list, "NTARF");
  if (tarif) {
    strncpy(d.tarif, tarif, sizeof(d.tarif) - 1);
    d.tarif[sizeof(d.tarif) - 1] = '\0';
  }

  int v = 0;
  if (!readLabelInt(list, "IRMS1", v) && !readLabelInt(list, "IINST1", v))
    readLabelInt(list, "IINST", v);
  d.current[0] = v;

  if (readLabelInt(list, "IRMS2", v) || readLabelInt(list, "IINST2", v))
    d.current[1] = v;
  if (readLabelInt(list, "IRMS3", v) || readLabelInt(list, "IINST3", v))
    d.current[2] = v;

  if (!readLabelInt(list, "URMS1", v)) readLabelInt(list, "URMS", v);
  if (v > 0) d.voltage[0] = v;
  if (readLabelInt(list, "URMS2", v) && v > 0) d.voltage[1] = v;
  if (readLabelInt(list, "URMS3", v) && v > 0) d.voltage[2] = v;

  d.totalPowerOut = -1;
  d.totalPowerIn = -1;
  d.totalPowerApparent = -1;
  for (size_t i = 0; i < 3; ++i) {
    d.hasPowerOutPhase[i] = false;
    d.hasPowerInPhase[i] = false;
  }

  if (readLabelInt(list, "SINSTS", v)) d.totalPowerOut = v;
  if (readLabelInt(list, "SINSTI", v)) d.totalPowerIn = v;
  if (readLabelInt(list, "PAPP", v)) d.totalPowerApparent = v;

  const char* outLabels[] = {"SINSTS1", "SINSTS2", "SINSTS3"};
  const char* inLabels[] = {"SINSTI1", "SINSTI2", "SINSTI3"};
  for (size_t i = 0; i < 3; ++i) {
    if (readLabelInt(list, outLabels[i], v)) {
      d.phasePowerOut[i] = v;
      d.hasPowerOutPhase[i] = true;
    }
    if (readLabelInt(list, inLabels[i], v)) {
      d.phasePowerIn[i] = v;
      d.hasPowerInPhase[i] = true;
    }
  }

  int64_t lv = 0;
  bool baseRead = false;
  if (readLabelInt64(list, "BASE", lv)) {
    d.indexHC = lv / 100;
    d.indexHP = 0;
    baseRead = true;
  }
  if (readLabelInt64(list, "EASF01", lv) || readLabelInt64(list, "HCHC", lv))
    d.indexHC = lv / 100;
  if (!baseRead && (readLabelInt64(list, "EASF02", lv) || readLabelInt64(list, "HCHP", lv)))
    d.indexHP = lv / 100;
  if (readLabelInt64(list, "EAIT", lv))
    d.indexInj = lv / 100;

  recomputeDerived(d);
  updateProtocol(list);

  data_ = d;
  lastValidFrameMs_ = millis();
}

void TeleinfoReader::recomputeDerived(TeleinfoData& d) {
  int sumOutPhase = 0, sumInPhase = 0;
  bool hasOutPhase = false, hasInPhase = false;

  for (size_t i = 0; i < 3; ++i) {
    if (d.hasPowerOutPhase[i]) { hasOutPhase = true; sumOutPhase += d.phasePowerOut[i]; }
    if (d.hasPowerInPhase[i]) { hasInPhase = true; sumInPhase += d.phasePowerIn[i]; }
  }

  int totalOut = d.totalPowerOut >= 0 ? d.totalPowerOut : (hasOutPhase ? sumOutPhase : -1);
  int totalIn = d.totalPowerIn >= 0 ? d.totalPowerIn : (hasInPhase ? sumInPhase : -1);
  if (totalOut < 0 && d.totalPowerApparent >= 0) totalOut = d.totalPowerApparent;

  const bool hasInjection = totalIn > 0;

  if (hasInjection)
    d.totalPowerInst = -roundTo50(totalIn);
  else if (totalOut >= 0)
    d.totalPowerInst = roundTo50(totalOut);
  else
    d.totalPowerInst = 0;

  bool hasInstantPhasePower = false;
  for (size_t i = 0; i < 3; ++i) {
    d.phasePowerInst[i] = 0;
    const int phaseOut = d.hasPowerOutPhase[i] ? d.phasePowerOut[i] : 0;
    const int phaseIn = d.hasPowerInPhase[i] ? d.phasePowerIn[i] : 0;
    if (hasInjection && phaseIn > 0) {
      d.phasePowerInst[i] = -roundTo50(phaseIn);
      hasInstantPhasePower = true;
    } else if (!hasInjection && phaseOut > 0) {
      d.phasePowerInst[i] = roundTo50(phaseOut);
      hasInstantPhasePower = true;
    }
  }

  if (!hasInstantPhasePower && d.totalPowerInst != 0) {
    int sumCurrentAbs = 0;
    int currentAbs[3] = {0, 0, 0};
    for (size_t i = 0; i < 3; ++i) {
      currentAbs[i] = abs(d.current[i]);
      sumCurrentAbs += currentAbs[i];
    }
    if (sumCurrentAbs > 0) {
      int assigned = 0;
      for (size_t i = 0; i < 3; ++i) {
        if (i < 2) {
          d.phasePowerInst[i] = (d.totalPowerInst * currentAbs[i]) / sumCurrentAbs;
          assigned += d.phasePowerInst[i];
        } else {
          d.phasePowerInst[i] = d.totalPowerInst - assigned;
        }
      }
    }
  }

  for (size_t i = 0; i < 3; ++i) {
    if (hasInjection) {
      if (d.current[i] > 0) d.current[i] = -d.current[i];
    } else {
      if (d.current[i] < 0) d.current[i] = -d.current[i];
    }
  }
}

void TeleinfoReader::onNewFrame(ValueList* list) {
  if (instance_) instance_->parseFrame(list);
}

void TeleinfoReader::onUpdatedFrame(ValueList* list) {
  if (instance_) instance_->parseFrame(list);
}
