#pragma once
#include <Arduino.h>
#include "ShadowSound.h"


namespace Board {

static inline void initGPIO()
{
  pinMode(OUTPUT_ENABLE_PIN, OUTPUT);
  digitalWrite(OUTPUT_ENABLE_PIN, LOW);

  pinMode(RS485_RTS_PIN, OUTPUT);
  digitalWrite(RS485_RTS_PIN, LOW);
}

static inline void initMaestroSerial(uint32_t baud)        { MAESTRO_SERIAL_INIT(baud); }
static inline void initBodyMaestroSerial(uint32_t baud)    { BODY_MAESTRO_SERIAL_INIT(baud); }
static inline void initMotorSerial(uint32_t baud)          { MOTOR_SERIAL_INIT(baud); }

static inline void initSoundSerial(SDSound::Module mod)
{
  uint32_t baud = SDSound::baudFor(mod);
  if (baud == 0) return;
  SOUND_SERIAL.end();
  SOUND_SERIAL_INIT(baud);
}

static inline void setOutputsEnabled(bool enabled) {
  digitalWrite(OUTPUT_ENABLE_PIN, enabled ? HIGH : LOW);
}

struct Periodic {
  uint32_t intervalMs;
  uint32_t nextDue;
  explicit Periodic(uint32_t ms) : intervalMs(ms), nextDue(0) {}
  template<typename F>
  inline void runIfDue(F&& fn) {
    uint32_t now = millis();
    if ((int32_t)(now - nextDue) >= 0) {
      fn();
      nextDue = now + intervalMs;
    }
  }
};

} // namespace Board
