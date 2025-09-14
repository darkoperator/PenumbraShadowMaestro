# Penumbra Shadow Maestro

A modernized fork of the original **SHADOW_MD** sketch, adapted for the [Penumbra Controller](https://github.com/reeltwo/PenumbraShadowMD) with:

- Support for **Pololu Maestro** boards (Dome & Body)
- Unified sound backend: SparkFun MP3 Trigger, DFPlayer Mini, DY-SV5W, HCR Vocalizer
- Flat random-track mode (no banks)
- Startup sound & random playback control
- NeoPixel strip support on GPIO 13
- All commands now accept parameters **without a space** (`#CMD42`) or **with a space** (`#CMD 42`)

---

## Libraries Used

- [Reeltwo](https://github.com/reeltwo/Reeltwo)  
- [espsoftwareserial](https://github.com/rimim/espsoftwareserial)  
- [USB_Host_Shield_2.0](https://github.com/reeltwo/USB_Host_Shield_2.0)  
- [DFRobotDFPlayerMini](https://github.com/DFRobot/DFRobotDFPlayerMini)  
- [Adafruit NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel)

> Add the Adafruit NeoPixel library to your `platformio.ini`:
```ini
lib_deps =
    adafruit/Adafruit NeoPixel
```

---

## Build

### Arduino IDE
Use ESP32 board package `1.0.6` or `2.0.5` (for ESP32-S2, `2.0.5` is required).

### PlatformIO / Command Line

**Mac:**
```bash
git clone https://github.com/reeltwo/PenumbraShadowMD
cd PenumbraShadowMD
pio run -e penumbrashadow
```

**Linux:**
```bash
git clone https://github.com/reeltwo/PenumbraShadowMD
cd PenumbraShadowMD
pio run -e penumbrashadow
```

---

## Wiring

See [Penumbra wiring diagram](https://user-images.githubusercontent.com/16616950/222179232-cd7f6191-de23-43d3-b792-a73715196444.png)

- Dome Maestro: `Serial1`
- Body Maestro: `Serial2`
- Sound cards (MP3 Trigger, DFPlayer, DY-SV5W) share `SOUND_SERIAL` (pins defined in `pin-map.h`).
- NeoPixels: GPIO 13.

---

## Command Reference

> All commands support **Option B**: value immediately after the command (`#CMD42`) or separated by space (`#CMD 42`).

### Core
| Command | Description |
|---------|-------------|
| `#SMHELP` | Show command help |
| `#SMDUMP` | Print restorable commands for current config |
| `#SMRESTART` | Reboot controller |
| `#SMZERO` | Clear preferences & unpair |
| `#SMLIST` | List button mappings |
| `#SMDEL<trigger>` | Reset a trigger |
| `#SMPLAY<trigger>` | Run the trigger’s action |
| `#SMSET<trigger> <action>` | Set trigger action (Marcduino/Meastro/Sound) |

#### Examples
- Bind a button to a Dome sequence: `#SMSET FTbtnUP_MD "DM58"`
- Bind a button to a Body sequence: `#SMSET btnUP_MD "BM2"`
- Run a sequence with sound: `#SMSET FTbtnUP_MD "DM58;S3"`
- Play only a track on press: `#SMSET btnRight_MD "S 42"`
- Run a sequence with random range: `#SMSET FTbtnUP_MD "DM58;S R 10 25"`
- Play a track immediately: `#SMPLAY 42`
 - Play a track immediately: `#SMPLAY 42` (DY supports up to 65535 → 00042.mp3)
 - Play a track immediately: `#SMPLAY 42` (DY supports up to 65535 → 00042.mp3)
- One-shot random from range: `#SMPLAYRAND 10 25` (or bind: `#SMSET btnX "S R 10 25"`)
- Persistent random range + enable: `#SMRANDTRACKS 10 25` then `#SMRAND1`

### Sound
| Command | Description |
|---------|-------------|
| `#SMSOUND0` | Disable sound |
| `#SMSOUND1` | Use MP3 Trigger |
| `#SMSOUND2` | Use DFPlayer Mini |
| `#SMSOUND3` | Use DY-SV5W |
| `#SMVOLUME0..1000` | Set volume (0 = mute, 1000 = max) |
| `#SMSTARTUP<n>` | Set startup track |
| `#SMRANDMIN<n>` | Min ms between random sounds |
| `#SMRANDMAX<n>` | Max ms between random sounds |
| `#SMRAND0` | Disable random playback |
| `#SMRAND1` | Enable random playback |
| `#SMRANDTRACKS a b` | (New) limit random sounds to tracks *a..b* |
| `#SMCONFIG` | Show drive/sound config |
| `#SMCARD` | Check audio/card status (DY sends test <P00001>) |
| `#SMCARD` | Check audio/card status (DY sends test <P00001>) |

### NeoPixels
| Command | Description |
|---------|-------------|
| `#SMNEOON` | Enable strip |
| `#SMNEOOFF` | Disable strip |
| `#SMNEOCOUNT<n>` | Set number of LEDs |
| `#SMNEOCOLOR R G B` | Set strip color |

### Drive / Dome
| Command | Description |
|---------|-------------|
| `#SMNORMALSPEED<n>` | Normal drive speed |
| `#SMMAXSPEED<n>` | Max over-throttle speed |
| `#SMTURNSPEED<n>` | Turn speed |
| `#SMDOMESPEED<n>` | Dome motor speed |
| `#SMRAMPING<n>` | Accel/decel ramp |
| `#SMFOOTDB<n>` | Foot joystick deadband |
| `#SMDOMEDB<n>` | Dome joystick deadband |
| `#SMDRIVEDB<n>` | Motor deadband |
| `#SMINVERT0/1` | Invert turn direction |
| `#SMAUTOSPEED<n>` | Dome auto speed |
| `#SMAUTOTIME<n>` | Dome 360° time (ms) |
| `#SMMOTORBAUD<n>` | Motor serial baud |
| `#SMMARCBAUD<n>` | Maestro serial baud |
| `#SMESPMAC` | Show this ESP32 WiFi MAC (ESP-NOW) |
| `#SMDOMEESP<0|1>` | Disable/Enable Dome Maestro via ESP-NOW |
| `#SMDOMEPEER <mac>` | Set Dome ESP-NOW peer MAC |
| `#SMDOMEENC<0|1>` | Disable/Enable ESP-NOW encryption with Dome peer |
| `#SMDOMEKEY <32HEX>` | Set ESP-NOW LMK (16 bytes as 32 hex chars) |
| `#SMPAIR` | Send simple pair request to Dome peer |

---

## Notes

- Startup sound plays after `sMarcSound.begin()` if volume is > 0.05.
- Random playback uses:
  - **Delay range** = `#SMRANDMIN` .. `#SMRANDMAX`
  - **Track range** = `#SMRANDTRACKS` (or full 1-255 if not set)
- SparkFun MP3 Trigger has **no banks**; DFPlayer & DY-SV5W do but the new “flat” random mode plays by number for all.

## Backup & Restore

- Export your current configuration as restorable commands with `#SMDUMP`.
- Save the output. After a factory reset (`#SMZERO`) or board swap, paste the dumped lines into the console to restore all settings and trigger bindings.

---

### Maintainer
**Carlos Perez**  
[carlos_perez@darkoperator.com](mailto:carlos_perez@darkoperator.com)  
Last Updated: 2025-09-11
