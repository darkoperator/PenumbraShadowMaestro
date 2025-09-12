#pragma once

#include <Arduino.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "DFRobotDFPlayerMini.h"

// -----------------------------------------------------------------------------
// Safety/debug macro guards so this header compiles regardless of include order
// -----------------------------------------------------------------------------
#ifndef DEBUG_PRINTLN
#define DEBUG_PRINTLN(...) do{}while(0)
#endif

#ifndef SOUND_DEBUG
#define SOUND_DEBUG(...)   do{}while(0)
#endif

// -----------------------------------------------------------------------------
// General constants (kept compatible with existing action strings)
// -----------------------------------------------------------------------------
#define MP3_MAX_BANKS                9   // banks 1..9 (legacy model)
#define MP3_MAX_SOUNDS_PER_BANK     25
#define MP3_BANK_CUTOFF             4

// Legacy bank sizes used only for banked → flat mapping in playSound()
#define MP3_BANK1_SOUNDS 19
#define MP3_BANK2_SOUNDS 18
#define MP3_BANK3_SOUNDS 7
#define MP3_BANK4_SOUNDS 4
#define MP3_BANK5_SOUNDS 3
#define MP3_BANK6_SOUNDS MP3_MAX_SOUNDS_PER_BANK
#define MP3_BANK7_SOUNDS MP3_MAX_SOUNDS_PER_BANK
#define MP3_BANK8_SOUNDS MP3_MAX_SOUNDS_PER_BANK
#define MP3_BANK9_SOUNDS MP3_MAX_SOUNDS_PER_BANK

#define MP3_EMPTY_SOUND 252  // used to stop on MP3 Trigger

// DFPlayer volume mapping
#define DF_VOLUME_MIN 1
#define DF_VOLUME_MAX 30
#define DF_VOLUME_OFF 0

// SparkFun MP3 Trigger volume mapping (0 is loudest; 254 quiet)
#define MP3_VOLUME_MIN 100
#define MP3_VOLUME_MAX 0
#define MP3_VOLUME_OFF 254
#define MP3_VOLUME_STEPS 20

// SparkFun MP3 Trigger serial command bytes
#define MP3_PLAY_CMD   't'
#define MP3_VOLUME_CMD 'v'

// Random scheduling defaults (ms)
#define MP3_MIN_RANDOM_PAUSE 600
#define MP3_MAX_RANDOM_PAUSE 10000

// Utility
#ifndef SizeOfArray
#define SizeOfArray(a) (sizeof(a)/sizeof((a)[0]))
#endif

// Local, non-conflicting prefix version of startswith to avoid symbol clashes
static inline bool ms_startswith(const char* str, const char* start) {
  size_t len = strlen(start);
  return strncasecmp(str, start, len) == 0;
}

class SDSound
{
public:
    enum Module {
        kDisabled = 0,
        kMP3Trigger = 1,
        kDFMini     = 2,
        kHCR        = 3
    };

    // Random timing (ms) – keep old API so your .ino compiles unchanged
    void setRandomMin(uint32_t ms) { fRandomMinDelay = ms; }
    void setRandomMax(uint32_t ms) { fRandomMaxDelay = ms; }

    // Keep your main loop calling this periodically
    void idle() {
        if (fModule == kDisabled) return;
        uint32_t now = millis();
        if (fRandomEnabled && fNextRandomEvent && now >= fNextRandomEvent) {
            playRandom();
            fNextRandomEvent = millis() + random(fRandomMinDelay, fRandomMaxDelay);
        }
    }

    // ---- Flat random track range (applies to all modules) ----
    void setRandomTracks(uint16_t lo, uint16_t hi) {
        if (lo < 1) lo = 1;
        if (hi < lo) hi = lo;
        if (hi > 255) hi = 255; // clamp globally (per-backend can clamp tighter)
        fRandMinTrack = lo;
        fRandMaxTrack = hi;
    }
    void getRandomTracks(uint16_t& lo, uint16_t& hi) const {
        lo = fRandMinTrack; hi = fRandMaxTrack;
    }

    // Canonical flat play; every backend gets a simple "track N"
    void playTrack(uint16_t flat) {
        if (flat < 1 || fModule == kDisabled) return;

        switch (fModule) {
            case kMP3Trigger:  sendMP3TriggerPlay(flat);   break; // 1..255
            case kDFMini:      sendDFPlayerPlayFlat(flat); break; // 1..2999
            case kHCR:
                // HCR is "mode" oriented; if you want HCR-by-track, map here.
                // For now, leave special behavior to handleCommand() paths.
                // You can replace with a numeric mapping if desired.
                break;
            default:           sendDYSV5WPlay(flat);       break; // DY-SV5W numeric
        }
    }

    // Legacy banked entrypoint; we keep it for older action strings.
    // It flattens bank/track to a single index before playing.
    void playSound(uint8_t bank, uint8_t sound) {
        if (bank > MP3_MAX_BANKS) return;
        if (bank != 0 && sound > MP3_MAX_SOUNDS_PER_BANK) return;

        // bank==0 means flat "sound" is already a global index
        if (bank == 0) {
            playTrack(sound);
            return;
        }

        uint8_t finalSound = sound;
        if (sound == 0) {
            // next/first logic for early banks, first-only for higher banks
            if (bank <= MP3_BANK_CUTOFF) {
                if ((++fBankIndexes[bank]) > fMaxSounds[bank]) fBankIndexes[bank] = 1;
                finalSound = fBankIndexes[bank];
            } else {
                finalSound = 1;
            }
        } else {
            // track set explicitly; clamp & store index
            if (sound > fMaxSounds[bank]) fBankIndexes[bank] = fMaxSounds[bank];
            else                          fBankIndexes[bank] = sound;
        }

        // Flatten bank + finalSound into a single 1-based number:
        uint16_t flat = flattenBankTrack(bank, finalSound);

        // Route to per-backend implementation
        playTrack(flat);
    }

    void playRandom() {
        if (fModule == kDisabled) return;

        // HCR "random" is a special chat macro in the legacy code; keep that behavior
        if (fModule == kHCR) {
            sendHCR("<MM>");
            return;
        }

        uint16_t lo = fRandMinTrack, hi = fRandMaxTrack;
        if (lo < 1) lo = 1;
        if (hi < lo) hi = lo;
        uint16_t pick = (uint16_t)random((int)lo, (int)hi + 1);
        playTrack(pick);
    }

    void stop() {
        switch (fModule) {
            case kDisabled: break;
            case kDFMini:   fDFMini.stop(); break;
            case kMP3Trigger:
                // Send a "stop" by playing the empty/stop track
                playSound(0, MP3_EMPTY_SOUND);
                break;
            case kHCR:
                sendHCR("<PSG>");
                break;
        }
    }

    inline void startRandom() { startRandomInSeconds(1); }
    void startRandomInSeconds(uint32_t seconds) {
        fRandomEnabled = true;
        fNextRandomEvent = millis() + seconds * 1000UL;
    }
    void suspendRandom() {
        fRandomEnabled = false;
        fNextRandomEvent = 0;
    }
    void resumeRandomInSeconds(uint32_t seconds) {
        fRandomEnabled = true;
        fNextRandomEvent = millis() + seconds * 1000UL;
    }
    void stopRandom() {
        fRandomEnabled = false;
        fNextRandomEvent = 0;
    }

    void volumeUp()   { setVolume(fVolume + (1.0f / MP3_VOLUME_STEPS)); }
    void volumeDown() { setVolume(fVolume - (1.0f / MP3_VOLUME_STEPS)); }

    void volumeOff() {
        switch (fModule) {
            case kDisabled: break;
            case kDFMini:   fDFMini.volume(DF_VOLUME_OFF); fVolume = 0; break;
            case kMP3Trigger:
                if (fStream) { fStream->write(MP3_VOLUME_CMD); fStream->write((uint8_t)MP3_VOLUME_OFF); }
                fVolume = 0;
                break;
            case kHCR:      sendHCR("<PVV000>"); break;
        }
    }

    void setVolume(float volume) {
        if (volume < 0.0f) volume = 0.0f;
        if (volume > 1.0f) volume = 1.0f;
        fVolume = volume;

        switch (fModule) {
            case kDisabled: break;
            case kDFMini: {
                int v = int(volume * (DF_VOLUME_MAX - DF_VOLUME_MIN)) + DF_VOLUME_MIN;
                if (v < DF_VOLUME_MIN) v = DF_VOLUME_MIN;
                if (v > DF_VOLUME_MAX) v = DF_VOLUME_MAX;
                fDFMini.volume(v);
                break;
            }
            case kMP3Trigger: {
                if (fStream) {
                    int v = MP3_VOLUME_MIN + int((MP3_VOLUME_MAX - MP3_VOLUME_MIN) * volume);
                    if (v < 0) v = 0;
                    if (v > 254) v = 254;
                    fStream->write(MP3_VOLUME_CMD);
                    fStream->write((uint8_t)v);
                }
                break;
            }
            case kHCR: {
                // Set both A/B channels to same percentage
                char buffer[30];
                int pct = int(volume * 100);
                if (pct < 0) pct = 0; if (pct > 100) pct = 100;
                snprintf(buffer, sizeof(buffer), "<PVV100><PVA%d><PVB%d>", pct, pct);
                sendHCR(buffer);
                break;
            }
        }
    }

    // Return to a clean state (used before switching modules)
    void end() {
        fModule = kDisabled;
        fStream = nullptr;
    }

    // Human-readable helpers for your parser/UI
    static const char* moduleName(Module m) {
        switch (m) {
            case kDisabled:   return "Disabled";
            case kMP3Trigger: return "MP3Trigger";
            case kDFMini:     return "DFMini";
            case kHCR:        return "HCR Vocalizer";
        }
        return "Unknown";
    }
    static Module fromChoice(int choice) {
        switch (choice) {
            case 0: return kDisabled;
            case 1: return kMP3Trigger;
            case 2: return kDFMini;
            case 3: return kHCR;
            default: return kDisabled;
        }
    }
    static uint32_t baudFor(Module m) {
        switch (m) {
            case kDisabled:   return 0;
            case kMP3Trigger: return 38400;
            case kDFMini:     return 9600;
            case kHCR:        return 9600;
        }
        return 0;
    }

    // Initialize a module on a given Stream (S/W serial, H/W serial, etc.)
    bool begin(Module module, Stream& stream, int startupSound = -1) {
        fModule = kDisabled;
        fStartupSound = startupSound;
        fStream = &stream;

        switch (module) {
            case kDisabled:
                fStream = nullptr;
                break;

            case kDFMini:
                if (!fDFMini.begin(stream)) {
                    DEBUG_PRINTLN("DFMini begin failed");
                    return false;
                }
                fDFMini.EQ(DFPLAYER_EQ_NORMAL);
                break;

            case kMP3Trigger:
                // Nothing extra to init; just hold the stream
                break;

            case kHCR:
                // Mode init (twice per original code)
                sendHCR("<M0>");
                sendHCR("<M0>");
                break;
        }

        memset(fBankIndexes, 0, sizeof(fBankIndexes));
        fModule = module;
        return true;
    }


    // Returns true if the command was consumed.
    bool handleCommand(const char* cmd, bool /*skipStart*/ = false) {
        if (!cmd || *cmd != '$') return false;
        cmd++; // after '$'

        switch (fModule) {
            case kDisabled:   return false;

            case kDFMini:
            case kMP3Trigger:
                if (isdigit((unsigned char)*cmd)) {
                    // flat number after '$' (e.g., "$17")
                    unsigned long n = strtoul(cmd, nullptr, 10);
                    if (n >= 1 && n <= 255) playTrack((uint16_t)n);
                    return true;
                } else {
                    // small command set
                    switch (*cmd) {
                        case 'R': // short random burst
                            suspendRandom(); playRandom(); resumeRandomInSeconds(10); return true;
                        case 'E': // extra-long random burst
                            suspendRandom(); playRandom(); resumeRandomInSeconds(20); return true;

                        // Next-by-bank (legacy)
                        case 'G': case 'H': case 'I': case 'J': case 'K':
                            playSound((*cmd) - 'G' + 1, 0); return true;

                        // Specific-in-bank (legacy)
                        case 'g': case 'h': case 'i': case 'j': case 'k':
                            playSound((*cmd) - 'g' + 1, (uint8_t)strtoul(++cmd, nullptr, 10));
                            return true;

                        case 'V': // volume, expects 1..20 (R2 Touch style)
                            if (ms_startswith(cmd, "VV") || ms_startswith(cmd, "VD")) {
                                if (isdigit((unsigned char)cmd[2])) {
                                    uint8_t volSteps = (uint8_t)atoi(&cmd[2]);
                                    if (volSteps < 0) volSteps = 0;
                                    if (volSteps > MP3_VOLUME_STEPS) volSteps = MP3_VOLUME_STEPS;
                                    setVolume((float)volSteps / MP3_VOLUME_STEPS);
                                    return true;
                                }
                            }
                            return false;

                        case 'O': // off
                            stopRandom(); volumeOff(); return true;

                        default: return false;
                    }
                }

            case kHCR:
                // HCR legacy special cases
                switch (*cmd) {
                    case 'R': playRandom(); return true;
                    case 'F': suspendRandom(); playSound(0, (uint8_t)fStartupSound); return true;
                    case 'S': suspendRandom(); playSound(8,4); resumeRandomInSeconds(34); return true;
                    case 'W': stopRandom();   playSound(8,2); return true;
                    case 'M': stopRandom();   playSound(8,3); return true;
                    default:  return false;
                }
        }
        return false;
    }

private:
    // Random flat track range
    uint16_t fRandMinTrack = 1;
    uint16_t fRandMaxTrack = 255;

    // Legacy bank enum (for readability when mapping)
    enum Bank {
        kGenSounds     = 1,
        kChatSounds    = 2,
        kHappySounds   = 3,
        kSadSounds     = 4,
        kWhistleSounds = 5,
        kScreamSounds  = 6,
        kLeiaSounds    = 7,
        kSingSounds    = 8,
        kMusicSounds   = 9
    };

    // Current transport and module
    Stream* fStream = nullptr;
    Module  fModule = kDisabled;

    // Legacy bank meta
    uint8_t fMaxSounds[MP3_MAX_BANKS + 1] = {
        0,
        MP3_BANK1_SOUNDS, MP3_BANK2_SOUNDS, MP3_BANK3_SOUNDS, MP3_BANK4_SOUNDS,
        MP3_BANK5_SOUNDS, MP3_BANK6_SOUNDS, MP3_BANK7_SOUNDS, MP3_BANK8_SOUNDS, MP3_BANK9_SOUNDS
    };
    uint8_t fBankIndexes[MP3_MAX_BANKS + 1] = {0};

    int   fStartupSound = -1;
    float fVolume       = 0.5f;

    bool     fRandomEnabled   = false;
    uint32_t fRandomMinDelay  = MP3_MIN_RANDOM_PAUSE;
    uint32_t fRandomMaxDelay  = MP3_MAX_RANDOM_PAUSE;
    uint32_t fNextRandomEvent = 0;

    DFRobotDFPlayerMini fDFMini;

    // ---- Helpers -------------------------------------------------------------

    // bank/track → flat (1-based)
    uint16_t flattenBankTrack(uint8_t bank, uint8_t track) const {
        if (bank < 1 || bank > 9 || track < 1) return 0;
        uint16_t offset = 0;
        for (uint8_t b = 1; b < bank; ++b) offset += fMaxSounds[b];
        if (track > fMaxSounds[bank]) track = fMaxSounds[bank];
        return offset + track;
    }

    // Backend: SparkFun MP3 Trigger (flat track number 1..255)
    inline void sendMP3TriggerPlay(uint16_t track) {
        if (!fStream) return;
        if (track < 1) track = 1;
        if (track > 255) track = 255;
        fStream->write((uint8_t)MP3_PLAY_CMD); // 't'
        fStream->write((uint8_t)track);        // 1..255
    }

    // Backend: DFPlayer Mini (flat root index 1..2999)
    inline void sendDFPlayerPlayFlat(uint16_t track) {
        if (track < 1) track = 1;
        fDFMini.play(track);
    }

    // Backend: DY-SV5W (DY-V5W protocol – play track N)
    // Common "Play Index" frame: 7E 03 A2 <hi> <lo> EF
    inline void sendDYSV5WPlay(uint16_t track) {
        if (!fStream || track < 1) return;
        uint8_t hi = (track >> 8) & 0xFF;
        uint8_t lo = track & 0xFF;
        const uint8_t cmd[6] = {0x7E, 0x03, 0xA2, hi, lo, 0xEF};
        fStream->write(cmd, sizeof(cmd));
    }

    // Backend: HCR send
    inline void sendHCR(const char* s) {
        if (fStream) fStream->print(s);
    }
};
