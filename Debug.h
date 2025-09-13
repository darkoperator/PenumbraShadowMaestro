#pragma once
#include <Arduino.h>

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 1
#endif

#ifndef DEBUG_BAUD
#define DEBUG_BAUD 115200
#endif

#ifndef DEBUG_PORT
#define DEBUG_PORT Serial
#endif

#if DEBUG_LEVEL
  #ifndef DEBUG_BEGIN
    #define DEBUG_BEGIN()     do{ DEBUG_PORT.begin(DEBUG_BAUD); }while(0)
  #endif
  /* Intentionally DO NOT define DEBUG_PRINT to avoid conflicts with ReelTwo */
  #ifndef DEBUG_PRINTLN
    #define DEBUG_PRINTLN(...) do{ DEBUG_PORT.println(__VA_ARGS__); }while(0)
  #endif
  #ifndef SOUND_DEBUG
    #define SOUND_DEBUG(...)  do{ DEBUG_PORT.println(__VA_ARGS__); }while(0)
  #endif
#else
  #ifndef DEBUG_BEGIN
    #define DEBUG_BEGIN()     do{}while(0)
  #endif
  #ifndef DEBUG_PRINTLN
    #define DEBUG_PRINTLN(...) do{}while(0)
  #endif
  #ifndef SOUND_DEBUG
    #define SOUND_DEBUG(...)  do{}while(0)
  #endif
#endif
