#pragma once
#include <Arduino.h>

// Simple ESPNOW message for Dome Maestro control
// type = 1 => Run Dome Maestro subroutine <subIndex>
struct __attribute__((packed)) SmNowMsg {
  uint8_t type;
  uint8_t subIndex;
};

static const uint8_t kSmNowType_DomeSub = 1;

