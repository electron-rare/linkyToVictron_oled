#pragma once

#include <stdint.h>

enum class TeleinfoProtocol : uint8_t {
  Unknown,
  Standard,
  Historical,
};

struct TeleinfoData {
  int current[3] = {0, 0, 0};
  int voltage[3] = {230, 230, 230};

  int phasePowerOut[3] = {0, 0, 0};
  int phasePowerIn[3] = {0, 0, 0};
  bool hasPowerOutPhase[3] = {false, false, false};
  bool hasPowerInPhase[3] = {false, false, false};

  int totalPowerOut = -1;
  int totalPowerIn = -1;
  int totalPowerApparent = -1;

  int phasePowerInst[3] = {0, 0, 0};
  int totalPowerInst = 0;

  int64_t indexHC = 0;
  int64_t indexHP = 0;
  int64_t indexInj = 0;

  bool valid = false;
  char tarif[16] = "--";
};
