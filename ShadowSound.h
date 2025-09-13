#pragma once

#include <Arduino.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "DFRobotDFPlayerMini.h"
#include <SoftwareSerial.h>
#ifdef USE_DYPLAYER
// DYPlayer integration deferred; fallback binary protocol is used for DY-SV5W
#endif

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
#define MP3_MAX_BANKS                9   // matches legacy "banked" addressing idea
#define MP3_MIN_TRACK                1
#define MP3_MAX_TRACK                255

// Helper clamp
static inline uint16_t _clampU16(uint16_t v, uint16_t lo, uint16_t hi){
  if(v<lo) return lo; if(v>hi) return hi; return v;
}

// -----------------------------------------------------------------------------
// Sound driver
// -----------------------------------------------------------------------------
class SDSound {
public:
    enum Module {
        kDisabled   = 0,
        kMP3Trigger = 1,
        kDFMini     = 2,
        kDYSV5W     = 3
    };

    // Random timing (ms) – keep old API so your .ino compiles unchanged
    void setRandomMin(uint32_t ms)      { fRandMin = ms; }
    void setRandomMax(uint32_t ms)      { fRandMax = ms; }

    // Back-compat helpers expected by older sketches
    void getRandomTracks(uint16_t& lo, uint16_t& hi) const {
        lo = fRandLo; hi = fRandHi;
    }
    void playRandomTrack(uint16_t lo, uint16_t hi) {
        setRandomTracks(lo, hi);
        playTrack(_randFlat());
    }
    void setRandomTracks(uint16_t lo, uint16_t hi) {
        fRandLo = _clampU16(lo, MP3_MIN_TRACK, MP3_MAX_TRACK);
        fRandHi = _clampU16(hi, MP3_MIN_TRACK, MP3_MAX_TRACK);
        if (fRandLo > fRandHi) { uint16_t t=fRandLo; fRandLo=fRandHi; fRandHi=t; }
    }

    // Human-readable names for UI
    static const char* moduleName(Module m) {
        switch (m) {
            case kDisabled:   return "Disabled";
            case kMP3Trigger: return "MP3 Trigger";
            case kDFMini:     return "DFPlayer Mini";
            case kDYSV5W:     return "DY-SV5W";
            default:          return "Unknown";
        }
    }

    // Map SMSOUND# choice to module (0..3)
    static Module fromChoice(int choice) {
        switch (choice) {
            case 0: return kDisabled;
            case 1: return kMP3Trigger;
            case 2: return kDFMini;
            case 3: return kDYSV5W;
            default: return kDisabled;
        }
    }

    // Recommended baud for the module
    static uint32_t baudFor(Module m) {
        switch (m) {
            case kMP3Trigger: return 38400;
            case kDFMini:     return 9600;
            case kDYSV5W:     return 9600;
            default:          return 0;
        }
    }

    SDSound()
    : fModule(kDisabled),
      fStream(nullptr),
      fDF(),
      fRandMin(4000),
      fRandMax(12000),
      fRandLo(1),
      fRandHi(255),
      fRandomOn(false),
      fNextDue(0)
    {}

    // Bind to a module on a given Stream (HardwareSerial or SoftwareSerial)
    bool begin(Module module, Stream& stream, int startupSound=-1) {
        fModule = module;
        fStream = &stream;

        switch (fModule) {
            case kDisabled:
                break;

            case kMP3Trigger:
                // No explicit handshake required; assume correct baud set by caller
                break;

            case kDFMini: {
                // DFPlayer requires begin() on a SoftwareSerial/HardwareSerial.
                // We assume 'stream' is a HardwareSerial (DFRobot API takes Stream* internally).
                if (!fDF.begin(stream)) {
                    DEBUG_PRINTLN("DFMini: begin failed");
                    return false;
                }
                // Reasonable defaults
                fDF.volume(20); // ~50%
                break;
            }

            case kDYSV5W:
                // Use binary protocol fallback (no library dependency)
                break;
        }

        if (startupSound > 0) {
            delay(150); // let volume latch if the sketch set it prior
            playTrack((uint16_t)startupSound);
        }
        return true;
    }

    void end() {
        fModule = kDisabled;
        fStream = nullptr;
    }

    // Legacy "banked" API kept for compatibility. Banks are flattened to 1..255.
    void playSound(uint8_t bank, uint8_t track) {
        uint16_t flat = _flatten(bank, track);
        playTrack(flat);
    }

    // Flat addressing 1..255
    void playTrack(uint16_t flat) {
        flat = _clampU16(flat, MP3_MIN_TRACK, MP3_MAX_TRACK);
        switch (fModule) {
            case kMP3Trigger:  sendMP3TriggerPlay(flat);     break;
            case kDFMini:      sendDFPlayerPlayFlat(flat);   break;
            case kDYSV5W:
                sendDYSV5WPlay(flat);
                break;
            default:           break;
        }
    }

    // Optional helpers
    void playRandom() {
        switch (fModule) {
            case kMP3Trigger: playTrack(_randFlat()); break; // avoid device-side random cmd
            case kDYSV5W:     /* DY: handled in sketch by choosing a random index */ break;
            case kDFMini:     /* choose a random flat */ sendDFPlayerPlayFlat(_randFlat()); break;
            default: break;
        }
    }

    void stop() {
        switch (fModule) {
            case kMP3Trigger: /* no explicit stop cmd on Trigger; some firmwares use track 0 */ break;
            case kDYSV5W:     /* no explicit stop cmd; do nothing */ break;
            case kDFMini:     fDF.stop(); break;
            default: break;
        }
    }

    void volumeOff() {
        switch (fModule) {
            case kMP3Trigger:
                if (fStream) { fStream->write('v'); fStream->write((uint8_t)254); }
                break;
            case kDYSV5W:     sendDYSV5W("<PVV000>"); break;
            case kDFMini:     fDF.volume(0); break;
            default: break;
        }
    }

    // v in [0..1]
    void setVolume(float v) {
        if (v < 0) v = 0; if (v > 1) v = 1;
        switch (fModule) {
            case kMP3Trigger: {
                // Map 0..1 to Trigger byte (0 loudest..254 quiet): iv = (1-v)*254
                if (fStream) {
                    int iv = (int)lrintf((1.0f - v) * 254.0f);
                    if (iv < 0) iv = 0; if (iv > 254) iv = 254;
                    fStream->write('v');
                    fStream->write((uint8_t)iv);
                }
                break;
            }
            case kDYSV5W: {
                // ASCII compatibility path
                int iv = (int)lrintf(v * 30.0f);
                if (iv < 0) iv = 0; if (iv > 30) iv = 30;
                char buf[16];
                snprintf(buf, sizeof(buf), "V%03d", iv);
                fStream->print(buf); fStream->write('\r'); fStream->write('\n');
                snprintf(buf, sizeof(buf), "<V%03d>", iv);
                fStream->print(buf); fStream->write('\r'); fStream->write('\n');
                break;
            }
            case kDFMini: {
                int vol = (int)lrintf(v * 30.0f); // DF volume range 0..30
                if (vol < 0) vol = 0; if (vol > 30) vol = 30;
                fDF.volume(vol);
                break;
            }
            default: break;
        }
    }

    // Random scheduler (non-blocking). Call idle() periodically.
    void startRandom() {
        fRandomOn = true;
        _scheduleNext();
    }

    // Compatibility helper if your sketch calls startRandomInSeconds(n)
    void startRandomInSeconds(uint32_t seconds) {
        fRandomOn = true;
        uint32_t now = millis();
        fNextDue = now + seconds * 1000UL;
    }

    void stopRandom() {
        fRandomOn = false;
    }

    void idle() {
        if (!fRandomOn) return;
        uint32_t now = millis();
        if ((int32_t)(now - fNextDue) >= 0) {
            uint16_t pick = _randFlat();
            playTrack(pick);
            _scheduleNext();
        }
    }

    // Text action command handler (compatibility with your "#SM..." router)
    // Accepts things like: "$S:Vxxx", "$S:Pnnn" etc.  For your use now:
    //   "$S:P003" -> play 0003.mp3
    //   "$S:V500" -> volume 50%
    void handleCommand(const char* s) {
        if (!s) return;
        // Very small parser: look for S: commands
        const char* p = strstr(s, "$S:");
        if (!p) p = strstr(s, "$s:");
        if (!p) return;
        p += 3;
        if (toupper(*p) == 'P') {
            // Play <Pnnn>
            int n = atoi(p+1);
            if (n <= 0) n = 1;
            playTrack((uint16_t)n);
        } else if (toupper(*p) == 'V') {
            int iv = atoi(p+1);
            if (iv < 0) iv = 0; if (iv > 1000) iv = 1000;
            setVolume(iv / 1000.0f);
        }
    }

private:
    Module          fModule;
    Stream*         fStream;   // for MP3 Trigger & DY-SV5W raw prints
    DFRobotDFPlayerMini fDF;   // for DFPlayer
    // DYPlayer pointer intentionally omitted to avoid build-time coupling.

    uint32_t fRandMin;
    uint32_t fRandMax;
    uint16_t fRandLo;
    uint16_t fRandHi;
    bool     fRandomOn;
    uint32_t fNextDue;

    // ---- Utility -------------------------------------------------------------
    uint16_t _flatten(uint8_t bank, uint8_t track) const {
        // Banked addressing 1..9, track 1..255 -> flatten to 1..255
        // We keep it simple: bank 0 means "flat" already.
        if (bank == 0) return _clampU16(track, MP3_MIN_TRACK, MP3_MAX_TRACK);
        // If you previously mapped banks to ranges, adapt here.
        uint16_t flat = (uint16_t)track;
        return _clampU16(flat, MP3_MIN_TRACK, MP3_MAX_TRACK);
    }

    uint16_t _randFlat() const {
        // Use esp_random if available
        uint32_t r = (uint32_t)esp_random();
        uint16_t span = (uint16_t)(fRandHi - fRandLo + 1);
        return fRandLo + (uint16_t)(r % span);
    }

    void _scheduleNext() {
        uint32_t now = millis();
        uint32_t lo = fRandMin, hi = fRandMax;
        if (lo > hi) { uint32_t t=lo; lo=hi; hi=t; }
        uint32_t span = hi - lo + 1;
        uint32_t delta = (span == 0) ? lo : (lo + (esp_random() % span));
        fNextDue = now + delta;
    }

    // ---- MP3 Trigger helpers -------------------------------------------------
    inline void sendMP3TriggerPlay(uint16_t flat) {
        // MP3 Trigger: 't' followed by single byte track (1..255)
        if (!fStream) return;
        if (flat < 1) flat = 1; if (flat > 255) flat = 255;
        fStream->write('t');
        fStream->write((uint8_t)flat);
    }
    inline void sendMP3TriggerVolume(float v) {
        // Map 0..1 -> 0..100 (coarse)
        int iv = (int)lrintf(v * 100.0f);
        if (iv < 0) iv = 0; if (iv > 100) iv = 100;
        char buf[8];
        snprintf(buf, sizeof(buf), "v%03d", iv);
        fStream->println(buf);
    }

    // ---- DFPlayer helpers ----------------------------------------------------
    inline void sendDFPlayerPlayFlat(uint16_t flat) {
        // DFPlayer's folder/track addressing is different, but supports playNum
        fDF.play(flat);
    }

    // ---- DY-SV5W helpers -----------------------------------------------------
    inline void sendDYSV5WPlay(uint16_t flat) {
        // DY-SV5W binary: 7E 03 A2 <hi> <lo> EF (Play index)
        if (!fStream) return;
        if (flat < 1) flat = 1; if (flat > 255) flat = 255;
        uint8_t hi = (flat >> 8) & 0xFF;
        uint8_t lo = flat & 0xFF;
        const uint8_t frame[6] = {0x7E, 0x03, 0xA2, hi, lo, 0xEF};
        fStream->write(frame, sizeof(frame));
    }
    inline void sendDYSV5W(const char* s) {
        // Best-effort ASCII support for volume and misc; keep minimal
        if (!fStream || !s) return;
        fStream->print(s);
        fStream->write('\r'); fStream->write('\n');
    }
};
