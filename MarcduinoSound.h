#pragma once
#include <Arduino.h>
#include "DFRobotDFPlayerMini.h"

/*
 * MarcduinoSound.h  —  Penumbra (MarcDuino-less) sound driver
 *
 * Provides three UART back-ends:
 *   - DFPlayer Mini (UART)
 *   - DY-SV5W (UART 9600 8N1)
 *   - SparkFun MP3 Trigger (UART 38400 8N1, default TX=14, RX=35)
 *
 * Backward compatibility:
 *   - Keeps $-style command parser used in the sketch
 *   - Adds legacy enum aliases: kDFMini, kMP3Trigger, kHCR
 *   - Adds begin(Module, Stream&, int) dispatcher (no duplicate legacy cases)
 *   - Adds setRandomMin/Max used by preferences code
 *
 * Convenience for #SMSOUND:
 *   - fromChoice(int 0..3) → Module
 *   - baudFor(Module)      → 0(disabled), 9600, 38400
 *   - moduleName(Module)   → const char*
 *   - end(), isEnabled()
 *
 * IMPORTANT:
 *   - No MarcDuino/HCR traffic is sent anywhere.
 *   - Put ONE definition in your .ino:   MarcSound sMarcSound;
 */

#ifndef SOUND_DEBUG
  #define SOUND_DEBUG(...) do{}while(0)
#endif

// --------------------------
// Library-level configuration
// --------------------------
#define MP3_MAX_BANKS                9
#define MP3_MAX_SOUNDS_PER_BANK     25
#define MP3_BANK_CUTOFF             4   // banks <= this advance "next" on 0

// Per-bank sizes for random (adjust to your library)
#define MP3_BANK1_SOUNDS 19
#define MP3_BANK2_SOUNDS 18
#define MP3_BANK3_SOUNDS 7
#define MP3_BANK4_SOUNDS 4
#define MP3_BANK5_SOUNDS 3
#define MP3_BANK6_SOUNDS MP3_MAX_SOUNDS_PER_BANK
#define MP3_BANK7_SOUNDS MP3_MAX_SOUNDS_PER_BANK
#define MP3_BANK8_SOUNDS MP3_MAX_SOUNDS_PER_BANK
#define MP3_BANK9_SOUNDS MP3_MAX_SOUNDS_PER_BANK

// DFPlayer Mini volume scale
#define DF_VOLUME_MAX   30
#define DF_VOLUME_MIN   1

// SparkFun MP3 Trigger defaults
#define MP3TRIG_BAUD    38400  // from SparkFun docs
#define MP3TRIG_DEF_TX  14     // ESP32 pin request by user
#define MP3TRIG_DEF_RX  35     // ESP32 pin request by user

// DY-SV5W defaults
#define DYSV5W_BAUD     9600   // from DY-SV5W datasheet

// Random timing
#define MP3_VOLUME_STEPS 20
#define MP3_MIN_RANDOM_PAUSE 600
#define MP3_MAX_RANDOM_PAUSE 10000
#define MP3_MAX_PAUSE_ON_RESUME 12

class MarcSound
{
public:
    // -------------------- Back-end selection --------------------
    enum Module {
        kDisabled = 0,
        kDFMini_UART,
        kDY_SV5W_UART,
        kMP3Trigger_UART
    };

    // -------- Legacy enum aliases so existing code compiles unchanged --------
    static constexpr Module kDFMini     = kDFMini_UART;      // old name -> new
    static constexpr Module kMP3Trigger = kMP3Trigger_UART;  // old name -> new
    static constexpr Module kHCR        = kDY_SV5W_UART;     // Penumbra used "HCR" for non-DF UART

    MarcSound() : fModule(kDisabled) { resetBankIndexes(); }

    // -------------------- Helpers for #SMSOUND --------------------
    // Map 0..3 from UI to Module (0:Disabled, 1:MP3 Trigger, 2:DFMini, 3:DY-SV5W/HCR)
    static Module fromChoice(int choice) {
        switch (choice) {
            case 1: return kMP3Trigger_UART;
            case 2: return kDFMini_UART;
            case 3: return kDY_SV5W_UART;
            default: return kDisabled;
        }
    }

    // Report the UART baud required for a module (use to reopen SOUND_SERIAL correctly)
    static uint32_t baudFor(Module m) {
        switch (m) {
            case kMP3Trigger_UART: return MP3TRIG_BAUD; // 38400
            case kDFMini_UART:
            case kDY_SV5W_UART:    return DYSV5W_BAUD;  // 9600
            default:               return 0;            // disabled / none
        }
    }

    // Human-readable name (for #SMSTATUS or diagnostics)
    static const char* moduleName(Module m) {
        switch (m) {
            case kMP3Trigger_UART: return "MP3 Trigger";
            case kDFMini_UART:     return "DFMini Player";
            case kDY_SV5W_UART:    return "HCR Vocalizer (DY-SV5W)";
            default:               return "Disabled";
        }
    }

    bool isEnabled() const { return fModule != kDisabled; }

    // Stop and detach (lets you safely re-begin with a different backend)
    void end() {
        stop();
        fDFSerial   = nullptr;
        fDYSerial   = nullptr;
        fTrigSerial = nullptr;
        fModule     = kDisabled;
    }

    // -------------------- Begin() overloads --------------------

    // DFPlayer Mini (UART)
    bool beginDFPlayer(Stream& serial, int startupSound = -1) {
        fModule = kDisabled;
        fStartupSound = startupSound;
        fDFSerial = &serial;
        if (!fDFMini.begin(serial)) {
            SOUND_DEBUG("DFMini: begin() failed\n");
            return false;
        }
        fDFMini.EQ(DFPLAYER_EQ_NORMAL);
        resetBankIndexes();
        fModule = kDFMini_UART;
        return true;
    }

    // DY-SV5W (UART 9600 8N1)
    bool beginDYSV5W(Stream& serial, int startupSound = -1) {
        fModule = kDisabled;
        fStartupSound = startupSound;
        fDYSerial = &serial;
        resetBankIndexes();
        fModule = kDY_SV5W_UART;
        return true;
    }

    // SparkFun MP3 Trigger (UART) — convenience for ESP32 Serial2 on TX=14, RX=35
    bool beginMP3Trigger(HardwareSerial& hw = Serial2,
                         int txPin = MP3TRIG_DEF_TX,
                         int rxPin = MP3TRIG_DEF_RX,
                         int startupSound = -1)
    {
    #if defined(ARDUINO_ARCH_ESP32)
        hw.begin(MP3TRIG_BAUD, SERIAL_8N1, rxPin, txPin);
    #else
        (void)txPin; (void)rxPin; // not used on non-ESP32
        hw.begin(MP3TRIG_BAUD);
    #endif
        return beginMP3Trigger((Stream&)hw, startupSound);
    }

    // SparkFun MP3 Trigger with any Stream
    bool beginMP3Trigger(Stream& serial, int startupSound = -1) {
        fModule = kDisabled;
        fStartupSound = startupSound;
        fTrigSerial = &serial;
        resetBankIndexes();
        fModule = kMP3Trigger_UART;
        return true;
    }

    // -------- Back-compat dispatcher: begin(Module, Stream&, startupSound) --------
    // Note: legacy aliases resolve to the same underlying values as the canonical enums,
    // so we do NOT repeat legacy cases to avoid duplicate case labels.
    bool begin(Module module, Stream& serial, int startupSound = -1) {
      switch (module) {
        case kDFMini_UART:     return beginDFPlayer(serial, startupSound);
        case kDY_SV5W_UART:    return beginDYSV5W(serial, startupSound);
        case kMP3Trigger_UART: return beginMP3Trigger(serial, startupSound);
        default:               return false;
      }
    }

    // -------------------- Periodic --------------------
    void idle() {
        if (fModule == kDisabled) return;
        uint32_t now = millis();
        if (fRandomEnabled && fNextRandomEvent && now >= fNextRandomEvent) {
            playRandom();
            fNextRandomEvent = millis() + random(fRandomMinDelay, fRandomMaxDelay);
        }
    }

    // -------------------- Core controls --------------------
    void playStartSound() {
        if (fStartupSound != -1) playSound(0, fStartupSound);
    }

    void stop() {
        switch (fModule) {
            case kDisabled: break;
            case kDFMini_UART:     fDFMini.stop(); break;
            case kDY_SV5W_UART:    dy_cmd_stop();  break;
            case kMP3Trigger_UART: trig_cmd_stop(); break;
        }
    }

    // bank/sound => flattened track number (1..225)
    void playSound(uint8_t bank, uint8_t sound) {
        if (bank > MP3_MAX_BANKS) return;
        if (bank != 0 && sound > MP3_MAX_SOUNDS_PER_BANK) return;

        uint16_t fileNum = 0;
        if (bank == 0) {
            fileNum = sound; // direct (already 1-based)
        } else if (sound != 0) {
            fileNum = (bank - 1) * MP3_MAX_SOUNDS_PER_BANK + sound;
            fBankIndexes[bank] = min<uint8_t>(sound, fMaxSounds[bank]);
        } else {
            if (bank <= MP3_BANK_CUTOFF) {
                if ((++fBankIndexes[bank]) > fMaxSounds[bank]) fBankIndexes[bank] = 1;
                sound = fBankIndexes[bank];
            } else {
                sound = 1;
            }
            fileNum = (bank - 1) * MP3_MAX_SOUNDS_PER_BANK + sound;
        }

        switch (fModule) {
            case kDisabled: return;

            case kDFMini_UART:
                // DFPlayer: 1-based track in root
                fDFMini.play(fileNum);
                return;

            case kDY_SV5W_UART:
                // DY-SV5W: "play specified music" supports 1..65535
                dy_cmd_playTrack(fileNum);
                return;

            case kMP3Trigger_UART:
                // MP3 Trigger binary 't' command expects 0..255 index; map 1..225 -> 0..224
                if (fileNum > 0) trig_cmd_playBinary((uint8_t)(fileNum - 1));
                return;
        }
    }

    // -------------------- Randomization --------------------
    void playRandom() {
        if (fModule == kDisabled) return;
        uint8_t num = random(
            1, MP3_BANK1_SOUNDS + MP3_BANK2_SOUNDS + MP3_BANK3_SOUNDS + MP3_BANK4_SOUNDS + MP3_BANK5_SOUNDS
        );
        if (num <= MP3_BANK1_SOUNDS) { playSound(1, num); return; }
        num -= MP3_BANK1_SOUNDS;
        if (num <= MP3_BANK2_SOUNDS) { playSound(2, num); return; }
        num -= MP3_BANK2_SOUNDS;
        if (num <= MP3_BANK3_SOUNDS) { playSound(3, num); return; }
        num -= MP3_BANK3_SOUNDS;
        if (num <= MP3_BANK4_SOUNDS) { playSound(4, num); return; }
        num -= MP3_BANK4_SOUNDS;
        if (num <= MP3_BANK5_SOUNDS) { playSound(5, num); return; }
    }

    inline void startRandom()                  { startRandomInSeconds(1); }
    void startRandomInSeconds(uint32_t secs)   { fRandomEnabled = true; fNextRandomEvent = millis() + secs * 1000UL; }
    void stopRandom()                          { fRandomEnabledSaved = false; fRandomEnabled = false; }
    void suspendRandom()                       { fRandomEnabledSaved = fRandomEnabled; fRandomEnabled = false; }
    void resumeRandomInSeconds(uint32_t secs)  { fRandomEnabled = fRandomEnabledSaved; if (fRandomEnabled) fNextRandomEvent = millis() + secs * 1000UL; }
    inline void resumeRandom()                 { resumeRandomInSeconds(MP3_MAX_PAUSE_ON_RESUME); }

    // -------------------- Volume --------------------
    // DFPlayer: 1..30; DY-SV5W: 0..30; MP3 Trigger: 0(Loud)..240(Mute)
    void volumeMid()  { setVolume(0.5f); }
    void volumeOff()  { setVolume(0.0f); }
    void volumeMax()  { setVolume(1.0f); }
    void volumeMin()  { setVolume(0.01f); }
    void volumeUp()   { setVolume(fVolume + (1.0f / MP3_VOLUME_STEPS)); }
    void volumeDown() { setVolume(fVolume - (1.0f / MP3_VOLUME_STEPS)); }

    void setVolume(float volume) {
        volume = (volume < 0.0f) ? 0.0f : (volume > 1.0f ? 1.0f : volume);
        fVolume = volume;

        switch (fModule) {
            case kDisabled: break;

            case kDFMini_UART: {
                int v = (int)ceilf(volume * DF_VOLUME_MAX);
                if (v < DF_VOLUME_MIN && volume > 0.0f) v = DF_VOLUME_MIN;
                if (v > DF_VOLUME_MAX) v = DF_VOLUME_MAX;
                fDFMini.volume(v);
            } break;

            case kDY_SV5W_UART: {
                // 0..30 (20 default) per datasheet
                uint8_t v = (uint8_t)roundf(volume * 30.0f);
                if (v > 30) v = 30;
                dy_cmd_setVolume(v);
            } break;

            case kMP3Trigger_UART: {
                // Map 0.0..1.0 -> 240..0 (Trigger 0 loudest, 240 mute)
                uint8_t v = (uint8_t)roundf((1.0f - volume) * 240.0f);
                if (v > 240) v = 240;
                trig_cmd_setVolume(v);
            } break;
        }
    }

    // -------- Back-compat random range setters used by preferences --------
    void setRandomMin(uint32_t ms) { fRandomMinDelay = ms; }
    void setRandomMax(uint32_t ms) { fRandomMaxDelay = ms; }

    // -------------------- $-style compat parser --------------------
    // Keeps legacy $XYY shortcuts working against the selected module.
    bool handleCommand(const char* cmd, bool skipStart = false) {
        if (!cmd) return false;
        if (skipStart) cmd--;
        uint8_t len = strlen(cmd);
        if (!skipStart && cmd[0] != '$') return false;
        if (len < 2 || len > 4) return false;

        char c1 = cmd[1];

        if (isdigit(c1)) {
            stopRandom();
            uint8_t bank = (uint8_t)c1 - '0';
            uint8_t sound = (len > 2) ? (uint8_t)atoi(cmd + 2) : 0;
            playSound(bank, sound);
            return true;
        }

        switch (c1) {
            case 'R': startRandom(); break;
            case 'O': stopRandom(); volumeOff(); break;
            case 'L': suspendRandom(); playSound(7,1);  resumeRandomInSeconds(44); break;
            case 'C': suspendRandom(); playSound(8,5);  resumeRandomInSeconds(56); break;
            case 'c': suspendRandom(); playSound(8,1);  resumeRandomInSeconds(27); break;
            case 'S': suspendRandom(); playSound(6,2);  resumeRandom(); break;
            case 'F': suspendRandom(); playSound(6,3);  resumeRandom(); break;
            case 'D': suspendRandom(); playSound(8,6);  resumeRandomInSeconds(40); break;
            case 's': stopRandom();   stop();           break;
            case '+': volumeUp();     break;
            case '-': volumeDown();   break;
            case 'm': volumeMid();    break;
            case 'f': volumeMax();    break;
            case 'p': volumeMin();    break;
            case 'W': stopRandom();   playSound(8,2);   break;
            case 'M': stopRandom();   playSound(8,3);   break;
            default:  return false;
        }
        return true;
    }

private:
    // -------- Internal state --------
    DFRobotDFPlayerMini fDFMini;
    Stream*   fDFSerial   = nullptr; // DFPlayer UART
    Stream*   fDYSerial   = nullptr; // DY-SV5W  UART
    Stream*   fTrigSerial = nullptr; // MP3 Trigger UART

    Module    fModule = kDisabled;
    float     fVolume = 0.5f;

    bool      fRandomEnabled = false;
    bool      fRandomEnabledSaved = false;
    uint32_t  fNextRandomEvent = 0;
    uint32_t  fRandomMinDelay = MP3_MIN_RANDOM_PAUSE;
    uint32_t  fRandomMaxDelay = MP3_MAX_RANDOM_PAUSE;
    int       fStartupSound = -1;

    uint8_t   fBankIndexes[MP3_MAX_BANKS] {};
    const uint8_t fMaxSounds[MP3_MAX_BANKS] =
    {
        MP3_BANK1_SOUNDS, MP3_BANK2_SOUNDS, MP3_BANK3_SOUNDS,
        MP3_BANK4_SOUNDS, MP3_BANK5_SOUNDS, MP3_BANK6_SOUNDS,
        MP3_BANK7_SOUNDS, MP3_BANK8_SOUNDS, MP3_BANK9_SOUNDS
    };

    enum { kGenSounds=1, kChatSounds, kHappySounds, kSadSounds, kWhistleSounds, kScreamSounds, kLeiaSounds, kSingSounds, kMusicSounds };

    // -------- Helpers --------
    void resetBankIndexes() { for (uint8_t i=0; i<MP3_MAX_BANKS; ++i) fBankIndexes[i] = 0; }

    // ===== DY-SV5W UART protocol (9600 8N1) =====
    // Frame format used here: 0xAA, CMD, LEN, [DATA...], SM (sum of prior bytes, low 8 bits)
    void dy_send(uint8_t cmd, const uint8_t* data=nullptr, uint8_t len=0) {
        if (!fDYSerial) return;
        uint8_t sum = 0;
        auto put = [&](uint8_t b){ fDYSerial->write(b); sum += b; };
        put(0xAA); put(cmd); put(len);
        for (uint8_t i=0; i<len; ++i) put(data[i]);
        fDYSerial->write(sum); // SM
    }
    void dy_cmd_stop()                    { dy_send(0x04, nullptr, 0); }
    void dy_cmd_play()                    { dy_send(0x02, nullptr, 0); }
    void dy_cmd_pause()                   { dy_send(0x03, nullptr, 0); }
    void dy_cmd_prev()                    { dy_send(0x05, nullptr, 0); }
    void dy_cmd_next()                    { dy_send(0x06, nullptr, 0); }
    void dy_cmd_setVolume(uint8_t v0_30)  { uint8_t d[1]={ (uint8_t)(v0_30>30?30:v0_30) }; dy_send(0x13,d,1); }

    // Select specified music: AA 07 02 HI LO SM (track number 1..65535)
    void dy_cmd_playTrack(uint16_t track1based) {
        uint8_t d[2] = { (uint8_t)((track1based >> 8) & 0xFF), (uint8_t)(track1based & 0xFF) };
        dy_send(0x07, d, 2);
    }

    // ===== SparkFun MP3 Trigger UART (38400 8N1) =====
    // Minimal command set:
    //  't' + <bin index 0..255>  -> play track by index (binary)
    //  'O'                        -> stop
    //  'v' + <0..240>            -> volume (0 loudest .. 240 mute)
    void trig_write(uint8_t b) { if (fTrigSerial) fTrigSerial->write(b); }

    void trig_cmd_playBinary(uint8_t trackIndex0based) {
        if (!fTrigSerial) return;
        trig_write('t');           // binary trigger command
        trig_write(trackIndex0based);
    }
    void trig_cmd_stop() {
        if (!fTrigSerial) return;
        trig_write('O');           // Stop
    }
    void trig_cmd_setVolume(uint8_t vol0_loud_240_mute) {
        if (!fTrigSerial) return;
        if (vol0_loud_240_mute > 240) vol0_loud_240_mute = 240;
        trig_write('v');           // Set volume
        trig_write(vol0_loud_240_mute);
    }
};

// Only a declaration here (no inline variables → no C++17 requirement).
// Define ONE global instance in your .ino:
//   MarcSound sMarcSound;
extern MarcSound sMarcSound;
