// =======================================================================================
//                      Penumbra Shadow Maestro: Derived from SHADOW_MD
// =======================================================================================
//        SHADOW_MD:  Small Handheld Arduino Droid Operating Wand + Maestro Servo Control
// =======================================================================================
//                        Last Revised Date: 09/11/2025
//                            Revised By: Carlos Perez
//                     Contact: carlos_perez@darkoperator.com
//                        Previously Revised Date: 01/08/2023
//                            Revised By: skelmir
//                        Previously Revised Date: 08/23/2015
//                            Revised By: vint43
//                Inspired by the PADAWAN / KnightShade SHADOW effort
// =======================================================================================
//
//         This program is free software: you can redistribute it and/or modify it for
//         your personal use and the personal use of other astromech club members.  
//
//         This program is distributed in the hope that it will be useful 
//         as a courtesy to fellow astromech club members wanting to develop
//         their own droid control system.
//
//         IT IS OFFERED WITHOUT ANY WARRANTY; without even the implied warranty of
//         MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//         You are using this software at your own risk and, as a fellow club member, it is
//         expected you will have the proper experience / background to handle and manage that 
//         risk appropriately.  It is completely up to you to insure the safe operation of
//         your droid and to validate and test all aspects of your droid control system.
//
// =======================================================================================
//   Note: You will need an ESP32 with a USB Host MAX3421 chip.
// =======================================================================================
//
//   Required Libraries:
///
//   https://github.com/reeltwo/Reeltwo
//   https://github.com/rimim/espsoftwareserial
//   https://github.com/felis/USB_Host_Shield_2.0
//
// =======================================================================================
//
//   Sabertooth (Foot Drive):
//         Set Sabertooth 2x32 or 2x25 Dip Switches: 1 and 2 Down, All Others Up
//
//   SyRen 10 Dome Drive:
//         For SyRen packetized Serial Set Switches: 1, 2 and 4 Down, All Others Up
//
// =======================================================================================
//
//   Cytron SmartDriveDuo MDDS30 (Foot Drive):
//         Set Dip Switches 1,2,3 Up. 4,5,6 Down.
//
//   Cytron SmartDriveDuo MDDS10 (Dome Drive):
//         Set Dip Switches 1,2,3,6 Up. 4,5 Down.
//
//   Set Dip Switches 7 and 8 for your battery type. 0 Down 1 Up.
//      7:0 8:0 - LiPo
//      7:0 8:1 - NiMH
//      7:1 8:0 - SLA (Lead Acid)
//      7:1 8:1 - No battery monitoring
//
// =======================================================================================
//
// ---------------------------------------------------------------------------------------
//                        General User Settings
// ---------------------------------------------------------------------------------------

#define PANEL_COUNT 10                // Number of panels
#define USE_DEBUG                     // Define to enable debug diagnostic
#define USE_PREFERENCES
#define USE_SABERTOOTH_PACKET_SERIAL
//#define USE_CYTRON_PACKET_SERIAL
#define USE_MP3_TRIGGER
//#define USE_DFMINI_PLAYER
//#define USE_HCR_VOCALIZER

//For Speed Setting (Normal): set this to whatever speeds works for you. 0-stop, 127-full speed.
#define DEFAULT_DRIVE_SPEED_NORMAL          70
//For Speed Setting (Over Throttle): set this for when needing extra power. 0-stop, 127-full speed.
#define DEFAULT_DRIVE_SPEED_OVER_THROTTLE   100

// the higher this number the faster it will spin in place, lower - the easier to control.
// Recommend beginner: 40 to 50, experienced: 50+, I like 75
#define DEFAULT_TURN_SPEED                  50
                         
// If using a speed controller for the dome, sets the top speed. Use a number up to 127
#define DEFAULT_DOME_SPEED                  100
                         
// Ramping- the lower this number the longer R2 will take to speedup or slow down,
// change this by increments of 1
#define DEFAULT_RAMPING                     1
                 
// For controllers that centering problems, use the lowest number with no drift
#define DEFAULT_JOYSTICK_FOOT_DEADBAND      15

// For controllers that centering problems, use the lowest number with no drift
#define DEFAULT_JOYSTICK_DOME_DEADBAND      10

// Used to set the Sabertooth DeadZone for foot motors
#define DEFAULT_DRIVE_DEADBAND              10

//This may need to be set to true for some configurations
#define DEFAULT_INVERT_TURN_DIRECTION       false

// Speed used when dome automation is active - Valid Values: 50 - 100
#define DEFAULT_AUTO_DOME_SPEED             70

// milliseconds for dome to complete 360 turn at domeAutoSpeed - Valid Values: 2000 - 8000 (2000 = 2 seconds)
#define DEFAULT_AUTO_DOME_TURN_TIME         2500

// Motor serial communication baud rate. Default 9600
#define DEFAULT_MOTOR_BAUD                  9600

// Maestro serial communication baud rate. Default 9600
#define DEFAULT_MAESTRO_BAUD              9600

#define PS3_CONTROLLER_FOOT_MAC       "XX:XX:XX:XX:XX:XX"  //Set this to your FOOT PS3 controller MAC address
#define PS3_CONTROLLER_DOME_MAC       "XX:XX:XX:XX:XX:XX"  //Set to a secondary DOME PS3 controller MAC address (Optional)

String PS3ControllerFootMac = PS3_CONTROLLER_FOOT_MAC;
String PS3ControllerDomeMAC = PS3_CONTROLLER_DOME_MAC;

String PS3ControllerBackupFootMac = "XX";  //Set to the MAC Address of your BACKUP FOOT controller (Optional)
String PS3ControllerBackupDomeMAC = "XX";  //Set to the MAC Address of your BACKUP DOME controller (Optional)

byte drivespeed1 = DEFAULT_DRIVE_SPEED_NORMAL;
byte drivespeed2 = DEFAULT_DRIVE_SPEED_OVER_THROTTLE;
byte turnspeed = DEFAULT_TURN_SPEED;
byte domespeed = DEFAULT_DOME_SPEED;
byte ramping = DEFAULT_RAMPING;

byte joystickFootDeadZoneRange = DEFAULT_JOYSTICK_FOOT_DEADBAND;
byte joystickDomeDeadZoneRange = DEFAULT_JOYSTICK_DOME_DEADBAND;

byte driveDeadBandRange = DEFAULT_DRIVE_DEADBAND;

bool invertTurnDirection = DEFAULT_INVERT_TURN_DIRECTION;

byte domeAutoSpeed = DEFAULT_AUTO_DOME_SPEED;
int time360DomeTurn = DEFAULT_AUTO_DOME_TURN_TIME;


#define SHADOW_DEBUG(...)       //uncomment this for console DEBUG output
//#define SHADOW_VERBOSE(...)   //uncomment this for console VERBOSE output

// #define SHADOW_DEBUG(...) printf(__VA_ARGS__);
#define SHADOW_VERBOSE(...) printf(__VA_ARGS__);
#ifdef USE_PREFERENCES
#include "Debug.h"
#include "pin-map.h"
#include "ShadowSound.h"
#include "BoardInit.h"
SDSound sShadowSound;
#include <Preferences.h>
#define PREFERENCE_PS3_FOOT_MAC             "ps3footmac"
#define PREFERENCE_PS3_DOME_MAC             "ps3domemac"
#define PREFERENCE_SHADOWSOUND                "msound"
#define PREFERENCE_SHADOWSOUND_VOLUME         "mvolume"
#define PREFERENCE_SHADOWSOUND_STARTUP        "msoundstart"
#define PREFERENCE_SHADOWSOUND_RANDOM         "mrandom"
#define PREFERENCE_SHADOWSOUND_RANDOM_MIN     "mrandommin"
#define PREFERENCE_SHADOWSOUND_RANDOM_MAX     "mrandommax"
#define PREFERENCE_SPEED_NORMAL             "smspeednorm"
#define PREFERENCE_SPEED_OVER_THROTTLE      "smspeedmax"
#define PREFERENCE_TURN_SPEED               "smspeedturn"
#define PREFERENCE_DOME_SPEED               "smspeeddome"
#define PREFERENCE_RAMPING                  "smramping"
#define PREFERENCE_FOOTSTICK_DEADBAND       "smfootdband"
#define PREFERENCE_DOMESTICK_DEADBAND       "smdomedband"
#define PREFERENCE_DRIVE_DEADBAND           "smdrivedband"
#define PREFERENCE_INVERT_TURN_DIRECTION    "sminvertturn"
#define PREFERENCE_DOME_AUTO_SPEED          "smdomeautospeed"
#define PREFERENCE_DOME_DOME_TURN_TIME      "smdometurntime"
#define PREFERENCE_MOTOR_BAUD               "smmotorbaud"
#define PREFERENCE_MAESTRO_BAUD           "smmarcbaud"
Preferences preferences;
#endif

// =======================================================================================

// NEW prototypes
bool handleMaestroAction(const char* action);
void sendDomeMaestroSequence(uint8_t subIndex);
void sendBodyMaestroSequence(uint8_t subIndex);

// Back-compat shims (optional)
void sendMarcCommand(const char* cmd);
void sendBodyMarcCommand(const char* cmd);

class ShadowButtonAction
{
public:
    ShadowButtonAction(const char* name, const char* default_action) :
        fNext(NULL), fName(name), fDefaultAction(default_action)
    {
        if (*head() == NULL) *head() = this;
        if (*tail() != NULL) (*tail())->fNext = this;
        *tail() = this;
    }

    static ShadowButtonAction* findAction(String name)
    {
        for (ShadowButtonAction* btn = *head(); btn != NULL; btn = btn->fNext)
            if (name.equalsIgnoreCase(btn->name())) return btn;
        return nullptr;
    }


    static void listActions()
    {
        for (ShadowButtonAction* btn = *head(); btn != NULL; btn = btn->fNext)
            printf("%s: %s\n", btn->name().c_str(), btn->action().c_str());
    }

    // Export actions as re-applicable commands
    static void dumpActions()
    {
        for (ShadowButtonAction* btn = *head(); btn != NULL; btn = btn->fNext)
            printf("#SMSET %s %s\n", btn->name().c_str(), btn->action().c_str());
    }

    void reset()
    {
        String key = fName;
        if (key.length() > 15) key = key.substring(0, 15);
        preferences.remove(key.c_str());
    }

    void setAction(String newAction)
    {
        if (strlen(fName) > 15) {
            String key = String(fName).substring(0, 15);
            preferences.putString(key.c_str(), newAction);
        } else {
            preferences.putString(fName, newAction);
        }
    }

    void trigger()
    {
        SHADOW_VERBOSE("TRIGGER: %s\n", fName);
        handleMaestroAction(action().c_str());
    }

    String name()   { return fName; }
    
    String action()
    {
        // NVS keys are limited to 15 chars → use a truncated key when needed
        String key = fName;
        if (key.length() > 15) key = key.substring(0, 15);

        // Avoid NOT_FOUND spam: only call getString if the key exists
        if (preferences.isKey(key.c_str())) {
            return preferences.getString(key.c_str(), fDefaultAction);
        } else {
            return String(fDefaultAction);
        }
    }

private:
    ShadowButtonAction* fNext;
    const char* fName;
    const char* fDefaultAction;

    static ShadowButtonAction** head() { static ShadowButtonAction* sHead; return &sHead; }
    static ShadowButtonAction** tail() { static ShadowButtonAction* sTail; return &sTail; }
};

#define SHADOW_ACTION(var,act) ShadowButtonAction var(#var,act);



//----------------------------------------------------
// CONFIGURE: The FOOT Navigation Controller Buttons
//----------------------------------------------------

SHADOW_ACTION(btnUP_MD, "BM:0")
SHADOW_ACTION(btnLeft_MD, "BM:2")
SHADOW_ACTION(btnRight_MD, "BM:3")
SHADOW_ACTION(btnDown_MD, "BM:2")
SHADOW_ACTION(btnUP_L1_MD, "BM:5")
SHADOW_ACTION(btnLeft_L1_MD, "BM:6")
SHADOW_ACTION(btnRight_L1_MD, "BM:7")
SHADOW_ACTION(btnDown_L1_MD, "BM:8")
SHADOW_ACTION(btnUP_CROSS_MD, "BM:9")
SHADOW_ACTION(btnLeft_CROSS_MD, "BM:10")
SHADOW_ACTION(btnRight_CROSS_MD, "BM:11")
SHADOW_ACTION(btnDown_CROSS_MD, "BM:12")
SHADOW_ACTION(btnUP_CIRCLE_MD, "BM:13")
SHADOW_ACTION(btnLeft_CIRCLE_MD, "BM:14")
SHADOW_ACTION(btnRight_CIRCLE_MD, "BM:15")
SHADOW_ACTION(btnDown_CIRCLE_MD, "BM:16")
SHADOW_ACTION(btnUP_PS_MD, "BM:17")
SHADOW_ACTION(btnLeft_PS_MD, "BM:18")
SHADOW_ACTION(btnRight_PS_MD, "BM:19")
SHADOW_ACTION(btnDown_PS_MD, "BM:20")


//----------------------------------------------------
// CONFIGURE: The DOME Navigation Controller Buttons
//----------------------------------------------------
SHADOW_ACTION(FTbtnUP_MD, "DM:0")             // Arrow Up
SHADOW_ACTION(FTbtnLeft_MD, "DM:2")           // Arrow Left
SHADOW_ACTION(FTbtnRight_MD, "DM:3")          // Arrow Right
SHADOW_ACTION(FTbtnDown_MD, "DM:1")           // Arrow Down
SHADOW_ACTION(FTbtnUP_L1_MD, "DM:5")          // Arrow UP + L1
SHADOW_ACTION(FTbtnLeft_L1_MD, "DM:6")        // Arrow Left + L1
SHADOW_ACTION(FTbtnRight_L1_MD, "DM:7")       // Arrow Right + L1
SHADOW_ACTION(FTbtnDown_L1_MD, "DM:8")        // Arrow Down + L1
SHADOW_ACTION(FTbtnUP_CROSS_MD, "DM:9")       // Arrow UP + CROSS
SHADOW_ACTION(FTbtnLeft_CROSS_MD, "DM:10")    // Arrow Left + CROSS
SHADOW_ACTION(FTbtnRight_CROSS_MD, "DM:11")   // Arrow Right + CROSS
SHADOW_ACTION(FTbtnDown_CROSS_MD, "DM:12")    // Arrow Down + CROSS
SHADOW_ACTION(FTbtnUP_CIRCLE_MD, "DM:13")     // Arrow Up + CIRCLE
SHADOW_ACTION(FTbtnLeft_CIRCLE_MD, "DM:14")   // Arrow Left + CIRCLE
SHADOW_ACTION(FTbtnRight_CIRCLE_MD, "DM:15")  // Arrow Right + CIRCLE
SHADOW_ACTION(FTbtnDown_CIRCLE_MD, "DM:16")   // Arrow Down + CIRCLE
SHADOW_ACTION(FTbtnUP_PS_MD, "DM:17")         // Arrow UP + PS
SHADOW_ACTION(FTbtnLeft_PS_MD, "DM:18")       // Arrow Left + PS
SHADOW_ACTION(FTbtnRight_PS_MD, "DM:19")      // Arrow Right + PS
SHADOW_ACTION(FTbtnDown_PS_MD, "DM:20")       // Arrow Down + PS


// ---------------------------------------------------------------------------------------
//               SYSTEM VARIABLES - USER CONFIG SECTION COMPLETED
// ---------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------
//                          Drive Controller Settings
// ---------------------------------------------------------------------------------------

int motorControllerBaudRate = DEFAULT_MOTOR_BAUD;
int maestroBaudRate = DEFAULT_MAESTRO_BAUD;

#define FOOT_MOTOR_ADDR      128      // Serial Address for Foot Motor
#define DOME_MOTOR_ADDR      129      // Serial Address for Dome Motor

#ifdef DEBUG_PRINTLN
#undef DEBUG_PRINTLN
#endif

// ---------------------------------------------------------------------------------------
//                          Libraries
// ---------------------------------------------------------------------------------------
#include <ReelTwo.h>
#include <core/SetupEvent.h>
#include <core/StringUtils.h>

#include <PS3BT.h>

#include <usbhub.h>


// Satisfy IDE, which only needs to see the include statment in the ino.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif

#ifdef USE_SABERTOOTH_PACKET_SERIAL
#include <motor/SabertoothDriver.h>
#endif

#ifdef USE_CYTRON_PACKET_SERIAL
#include <motor/CytronSmartDriveDuoDriver.h>
#endif


#define CONSOLE_BUFFER_SIZE     300
static unsigned sPos;
static char sBuffer[CONSOLE_BUFFER_SIZE];
static bool sLastWasCR = false;

//---------------------------------------------------------------------------------------
//                          Command Parsing Helpers
//---------------------------------------------------------------------------------------

static inline void _skip_ws(char*& p) {
  while (*p == ' ' || *p == '\t') ++p;
}

// Works whether your existing startswith() advances cmd or not.
// After matching, it ensures `cmd` points to the first non-space character *after* the token.
static bool matchCmd(char*& cmd, const char* token) {
  if (!startswith(cmd, token)) return false;                   // may or may not advance
  size_t n = strlen(token);
  if (strncasecmp(cmd, token, n) == 0) cmd += n;               // if not advanced, advance now
  _skip_ws(cmd);                                               // allow optional whitespace
  return true;
}

// Parse a (signed) long integer argument right after the (optional-space) token.
// On success, advances cmd to the first non-digit that follows.
static bool parseLongArg(char*& cmd, long& out) {
  _skip_ws(cmd);
  if (!isdigit((unsigned char)*cmd) && *cmd != '-' && *cmd != '+') return false;
  char* endp = nullptr;
  long v = strtol(cmd, &endp, 10);
  if (endp == cmd) return false;
  cmd = endp;
  out = v;
  return true;
}

// Parse an unsigned int in range [min..max], print usage on failure (optional).
static bool parseUIntInRange(char*& cmd, uint32_t& out, uint32_t minV, uint32_t maxV,
                             const char* usage = nullptr) {
  long tmp = 0;
  if (!parseLongArg(cmd, tmp) || tmp < (long)minV || tmp > (long)maxV) {
    if (usage && *usage) printf("%s\n", usage);
    return false;
  }
  out = (uint32_t)tmp;
  return true;
}

// Convenience macro so branches read nicely:
#define CMD(TOK) else if (matchCmd(cmd, TOK))


// ---------------------------------------------------------------------------------------
//                    NeoPixels on Pin 13
// ---------------------------------------------------------------------------------------
// ----- NeoPixel State -----
#include <Adafruit_NeoPixel.h>        //Needed for the LED Control

#define NEOPIXEL_PIN   13

Adafruit_NeoPixel strip;
bool neopixelEnabled = false;
int neopixelCount    = 0;
uint8_t neopixelR    = 0;
uint8_t neopixelG    = 0;
uint8_t neopixelB    = 0;

// Initialize / reinitialize the strip
void initNeoPixels(int count)
{
    if (count <= 0) {
        strip.updateLength(0);
        neopixelCount = 0;
        return;
    }

    neopixelCount = count;
    strip.updateLength(neopixelCount);
    strip.setPin(NEOPIXEL_PIN);
    strip.begin();
    strip.show(); // clear
}

// Update strip with current color if enabled
void refreshNeoPixels()
{
    if (!neopixelEnabled || neopixelCount == 0) return;

    for (int i = 0; i < neopixelCount; i++) {
        strip.setPixelColor(i, strip.Color(neopixelR, neopixelG, neopixelB));
    }
    strip.show();
}

// ---------------------------------------------------------------------------------------
//                    Low-level Maestro helpers
// ---------------------------------------------------------------------------------------
bool handleSMSOUND(const char* bracketArg)
{
    if (!bracketArg) return false;
    int val = atoi(bracketArg);               // 0..3 from your UI
    SDSound::Module mod = SDSound::fromChoice(val);
    uint32_t baud = SDSound::baudFor(mod);

    // Persist choice
    preferences.putInt(PREFERENCE_SHADOWSOUND, (int)mod);

    // Re-init immediately (optional; or just apply next boot)
    sShadowSound.end();
    SOUND_SERIAL.end();
    if (baud) SOUND_SERIAL_INIT(baud);        // uses your pin-map.h macros
    if (baud) sShadowSound.begin(mod, SOUND_SERIAL, /*startupSound*/ -1);
    

    DEBUG_PRINTF("Sound module: %s (%d baud)\n", SDSound::moduleName(mod), (int)baud);
    return true;
}

static inline void maestroRestartAtSub(Stream& port, uint8_t subIndex)
{
    // Compact protocol: 0xA7 <subIndex>
    uint8_t pkt[2] = { 0xA7, subIndex };
    port.write(pkt, sizeof(pkt));
}

// Simple, non-blocking queueing for Maestro subroutine calls.
// Avoids overlapping DM/BM sequences by spacing calls by a cooldown.
static uint32_t kMaestroCooldownMs = 400;      // default spacing between sub restarts
static uint32_t domeSubBusyUntil = 0;
static int      domeSubPending   = -1;         // -1 = none
static uint32_t bodySubBusyUntil = 0;
static int      bodySubPending   = -1;         // -1 = none

// Query Maestro script running state (best-effort). Returns true if it appears running.
static bool maestroIsScriptRunning(Stream& port)
{
    // Compact Protocol: 0xAE = Get Script Status (returns 0 if not running, non-zero if running)
    // Best-effort, non-blocking: if no response yet, assume NOT running so we don't stall.
    port.write((uint8_t)0xAE);
    if (port.available() > 0) {
        int b = port.read();
        return b != 0;
    }
    return false; // no response available this instant → treat as not running
}

static inline void _sendDomeNow(uint8_t subIndex)
{
    maestroRestartAtSub(MAESTRO_SERIAL, subIndex);
    domeSubBusyUntil = millis() + kMaestroCooldownMs;
#ifdef SHADOW_VERBOSE
    SHADOW_VERBOSE("Dome Maestro: sub %u\n", subIndex);
#endif
}

static inline void _sendBodyNow(uint8_t subIndex)
{
    maestroRestartAtSub(BODY_MAESTRO_SERIAL, subIndex);
    bodySubBusyUntil = millis() + kMaestroCooldownMs;
#ifdef SHADOW_VERBOSE
    SHADOW_VERBOSE("Body Maestro: sub %u\n", subIndex);
#endif
}

void sendDomeMaestroSequence(uint8_t subIndex)
{
    uint32_t now = millis();
    if ((int32_t)(now - domeSubBusyUntil) >= 0 && !maestroIsScriptRunning(MAESTRO_SERIAL)) {
        _sendDomeNow(subIndex);
    } else {
        domeSubPending = (int)subIndex; // queue latest
    }
}

void sendBodyMaestroSequence(uint8_t subIndex)
{
    uint32_t now = millis();
    if ((int32_t)(now - bodySubBusyUntil) >= 0 && !maestroIsScriptRunning(BODY_MAESTRO_SERIAL)) {
        _sendBodyNow(subIndex);
    } else {
        bodySubPending = (int)subIndex; // queue latest
    }
}

static void pumpMaestroQueues()
{
    uint32_t now = millis();
    if (domeSubPending >= 0 && (int32_t)(now - domeSubBusyUntil) >= 0 && !maestroIsScriptRunning(MAESTRO_SERIAL)) {
        _sendDomeNow((uint8_t)domeSubPending);
        domeSubPending = -1;
    }
    if (bodySubPending >= 0 && (int32_t)(now - bodySubBusyUntil) >= 0 && !maestroIsScriptRunning(BODY_MAESTRO_SERIAL)) {
        _sendBodyNow((uint8_t)bodySubPending);
        bodySubPending = -1;
    }
}


// ---------------------------------------------------------------------------------------
//                          Variables
// ---------------------------------------------------------------------------------------

long previousDomeMillis = millis();
long previousFootMillis = millis();
long previousMarcDuinoMillis = millis();
long previousDomeToggleMillis = millis();
long previousSpeedToggleMillis = millis();
long currentMillis = millis();

int serialLatency = 25;   //This is a delay factor in ms to prevent queueing of the Serial data.
                          //25ms seems approprate for HardwareSerial, values of 50ms or larger are needed for Softare Emulation
                          
int marcDuinoButtonCounter = 0;
int speedToggleButtonCounter = 0;
int domeToggleButtonCounter = 0;

#ifdef USE_SABERTOOTH_PACKET_SERIAL
SabertoothDriver FootMotorImpl(FOOT_MOTOR_ADDR, MOTOR_SERIAL);
SabertoothDriver DomeMotorImpl(DOME_MOTOR_ADDR, MOTOR_SERIAL);

SabertoothDriver* FootMotor=&FootMotorImpl;
SabertoothDriver* DomeMotor=&DomeMotorImpl;
#endif

#ifdef USE_CYTRON_PACKET_SERIAL
CytronSmartDriveDuoMDDS30Driver FootMotorImpl(FOOT_MOTOR_ADDR, MOTOR_SERIAL);
CytronSmartDriveDuoMDDS10Driver DomeMotorImpl(DOME_MOTOR_ADDR, MOTOR_SERIAL);

CytronSmartDriveDuoDriver* FootMotor=&FootMotorImpl;
CytronSmartDriveDuoDriver* DomeMotor=&DomeMotorImpl;
#endif

///////Setup for USB and Bluetooth Devices////////////////////////////
USB Usb;
BTD Btd(&Usb);
PS3BT PS3NavFootImpl(&Btd);
PS3BT PS3NavDomeImpl(&Btd);
PS3BT* PS3NavFoot=&PS3NavFootImpl;
PS3BT* PS3NavDome=&PS3NavDomeImpl;

//Used for PS3 Fault Detection
uint32_t msgLagTime = 0;
uint32_t lastMsgTime = 0;
uint32_t currentTime = 0;
uint32_t lastLoopTime = 0;
int badPS3Data = 0;
int badPS3DataDome = 0;

bool firstMessage = true;

bool isFootMotorStopped = true;
bool isDomeMotorStopped = true;

bool overSpeedSelected = false;

bool isPS3NavigatonInitialized = false;
bool isSecondaryPS3NavigatonInitialized = false;

bool isStickEnabled = true;

bool WaitingforReconnect = false;
bool WaitingforReconnectDome = false;

bool mainControllerConnected = false;
bool domeControllerConnected = false;

// Dome Automation Variables
bool domeAutomation = false;
int domeTurnDirection = 1;  // 1 = positive turn, -1 negative turn
float domeTargetPosition = 0; // (0 - 359) - degrees in a circle, 0 = home
unsigned long domeStopTurnTime = 0;    // millis() when next turn should stop
unsigned long domeStartTurnTime = 0;  // millis() when next turn should start
int domeStatus = 0;  // 0 = stopped, 1 = prepare to turn, 2 = turning

byte action = 0;
unsigned long DriveMillis = 0;

int footDriveSpeed = 0;

// =======================================================================================

static inline bool _isSpace(char c){ return c==' '||c=='\t'||c=='\r'||c=='\n'; }

static String _trimCopy(const char* s)
{
    if (!s) return String();
    const char* b = s; while (*b && _isSpace(*b)) ++b;
    const char* e = b + strlen(b); while (e>b && _isSpace(*(e-1))) --e;
    return String(b).substring(0, e-b);
}

// Play a numeric sound track via your sound backend (bank 0=flat index)
static void _playTrackNumber(long n)
{
    if (n < 1) n = 1;
    if (n > 225) n = 225;
    sShadowSound.playSound(0, (uint8_t)n);
}

// Send a single-letter sound command as "$<LETTER>"
static void _soundLetterCmd(char letter)
{
    char buf[3] = {'$', (char)toupper((unsigned char)letter), 0};
    sShadowSound.handleCommand(buf);
}

// Parse "S..." after a semicolon; return true if something was executed.
static bool _handleSoundSuffix(const String& sVal)
{
    if (sVal.length() == 0) return false;

    // S R [min [max]] — one-shot random from range (or current configured range)
    if (toupper((unsigned char)sVal[0]) == 'R') {
        String rest = sVal.substring(1);
        rest.trim();
        long lo = -1, hi = -1;
        // Accept formats: "10 25", "10-25", "10" (lo==hi), or empty (use configured)
        if (rest.length() > 0) {
            // Replace '-' with space to normalize
            for (size_t i = 0; i < (size_t)rest.length(); ++i) {
                if (rest[i] == '-') rest.setCharAt(i, ' ');
            }
            char* endp = nullptr;
            lo = strtol(rest.c_str(), &endp, 10);
            while (endp && *endp && isspace((unsigned char)*endp)) ++endp;
            if (endp && *endp) hi = strtol(endp, nullptr, 10);
            if (lo < 1) lo = 1;
            if (hi < 1) hi = lo;
        }
        uint16_t rtlo = 1, rthi = 255;
        sShadowSound.getRandomTracks(rtlo, rthi);
        if (lo < 0) {
            lo = rtlo; hi = rthi;
        } else if (hi < 0) {
            hi = lo;
        }
        if (hi < lo) { long t = lo; lo = hi; hi = t; }
        if (lo < 1) lo = 1; if (hi > 255) hi = 255;
        sShadowSound.playRandomTrack((uint16_t)lo, (uint16_t)hi);
        return true;
    }

    // S<letter> ?
    if (sVal.length() == 1 && !isdigit((unsigned char)sVal[0])) {
        _soundLetterCmd(sVal[0]);
        return true;
    }

    // S<number> — allow decimal
    char* endp = nullptr;
    long n = strtol(sVal.c_str(), &endp, 10);
    if (endp && *endp == 0) {
        _playTrackNumber(n);
        return true;
    }

    // Mixed input? Take first char as command.
    _soundLetterCmd(sVal[0]);
    return true;
}

bool handleMaestroAction(const char* action)
{
    // Example accepted inputs:
    //   "DM58"              -> dome seq 58
    //   "BM12;S7"           -> body seq 12, then track 7
    //   "S f"               -> sound command $F
    //   "DM3;Sf, BM4"       -> (tokenized by commas) DM3+$F, then BM4

    String full = _trimCopy(action);
    if (full.length() == 0) return false;

    // Split by commas into tokens, process each
    int start = 0;
    bool any = false;
    while (start <= full.length()) {
        int comma = full.indexOf(',', start);
        String token = (comma < 0) ? full.substring(start) : full.substring(start, comma);
        token = _trimCopy(token.c_str());
        start = (comma < 0) ? (full.length() + 1) : (comma + 1);
        if (token.length() == 0) continue;

        // Split a token by optional ';' into LEFT(;RIGHT)
        int semi = token.indexOf(';');
        String left  = (semi < 0) ? token : token.substring(0, semi);
        String right = (semi < 0) ? String() : token.substring(semi + 1);

        // Normalize LEFT (DMxx / BMxx) and RIGHT (Sxx / Sx)
        left.trim(); right.trim();

        // Handle pure sound-only tokens like "S7" or "Sf"
        if (left.length() >= 1 && (left[0]=='S' || left[0]=='s') && (right.length()==0)) {
            String sval = left.substring(1); sval.trim();
            if (_handleSoundSuffix(sval)) any = true;
            continue;
        }

       // LEFT must be DMxx or BMxx (accept optional ':' or '=' separators)
        if (left.length() >= 3) {
            char c0 = toupper((unsigned char)left[0]);
            char c1 = toupper((unsigned char)left[1]);
            if ((c0=='D' || c0=='B') && c1=='M') {
                const char* p = left.c_str() + 2;
                // Skip optional separator and whitespace
                if (*p==':' || *p=='=') ++p;
                while (*p && _isSpace(*p)) ++p;

                char* endp = nullptr;
                long seq = strtol(p, &endp, 10);
                if (endp && seq >= 0 && seq <= 255) {
                    if (c0=='D') sendDomeMaestroSequence((uint8_t)seq);
                    else         sendBodyMaestroSequence((uint8_t)seq);
                    any = true;

                    // Optional RIGHT side "S.." (accept optional ':' or '=')
                    // Handle pure sound-only tokens like "S7", "S:7", "Sf", "S=f"
                    if (left.length() >= 1 && (left[0]=='S' || left[0]=='s') && (right.length()==0)) {
                        String sval = left.substring(1);
                        sval.trim();
                        if (sval.length() && (sval[0]==':' || sval[0]=='=')) {
                            sval.remove(0, 1);
                            sval.trim();
                        }
                        if (_handleSoundSuffix(sval)) any = true;
                        continue;
                    }
                    continue;
                }
            }
        }


        // If we reach here, token didn't match our formats — ignore gracefully.
        SHADOW_DEBUG("Unknown Maestro token \"%s\" in action \"%s\"\n", token.c_str(), action);
    }

    return any;
}

// =======================================================================================
//                          Main Program
// =======================================================================================

#if defined(USE_HCR_VOCALIZER) || defined(USE_MP3_TRIGGER) || defined(USE_DFMINI_PLAYER)
//#define SOUND_DEBUG(...) printf(__VA_ARGS__);
#define SHADOW_SOUND_VOLUME               500     // 0 - 1000
#define SHADOW_SOUND_RANDOM               true    // Play random sounds
#define SHADOW_SOUND_RANDOM_MIN           1000    // Min wait until random sound
#define SHADOW_SOUND_RANDOM_MAX           10000   // Max wait until random sound
#define SHADOW_SOUND_STARTUP              255     // Startup sound
#define SHADOW_SOUND_PLAYER               SDSound::kMP3Trigger  // Default sound module

#define SHADOW_SOUND
#endif

// =======================================================================================
//                          Initialize - Setup Function
// =======================================================================================
void setup()
{
    DEBUG_BEGIN();
    DEBUG_PRINTLN("\nBooting...");
    randomSeed(esp_random());
    Board::initGPIO();
    Board::setOutputsEnabled(false);

    REELTWO_READY();

#ifdef USE_PREFERENCES
    if (!preferences.begin("penumbrashadow", false))
    {
        DEBUG_PRINTLN("Failed to init prefs");
    }
    else
    {
        // ---- Load controller & motion prefs ----
        PS3ControllerFootMac       = preferences.getString(PREFERENCE_PS3_FOOT_MAC,      PS3_CONTROLLER_FOOT_MAC);
        PS3ControllerDomeMAC       = preferences.getString(PREFERENCE_PS3_DOME_MAC,      PS3_CONTROLLER_DOME_MAC);

        drivespeed1                = preferences.getInt(PREFERENCE_SPEED_NORMAL,          DEFAULT_DRIVE_SPEED_NORMAL);
        drivespeed2                = preferences.getInt(PREFERENCE_SPEED_OVER_THROTTLE,   DEFAULT_DRIVE_SPEED_OVER_THROTTLE);
        turnspeed                  = preferences.getInt(PREFERENCE_TURN_SPEED,            DEFAULT_TURN_SPEED);
        domespeed                  = preferences.getInt(PREFERENCE_DOME_SPEED,            DEFAULT_DOME_SPEED);
        ramping                    = preferences.getInt(PREFERENCE_RAMPING,               DEFAULT_RAMPING);
        joystickFootDeadZoneRange  = preferences.getInt(PREFERENCE_FOOTSTICK_DEADBAND,    DEFAULT_JOYSTICK_FOOT_DEADBAND);
        joystickDomeDeadZoneRange  = preferences.getInt(PREFERENCE_DOMESTICK_DEADBAND,    DEFAULT_JOYSTICK_DOME_DEADBAND);
        driveDeadBandRange         = preferences.getInt(PREFERENCE_DRIVE_DEADBAND,        DEFAULT_DRIVE_DEADBAND);
        invertTurnDirection        = preferences.getBool(PREFERENCE_INVERT_TURN_DIRECTION,DEFAULT_INVERT_TURN_DIRECTION);
        domeAutoSpeed              = preferences.getInt(PREFERENCE_DOME_AUTO_SPEED,       DEFAULT_AUTO_DOME_SPEED);
        time360DomeTurn            = preferences.getInt(PREFERENCE_DOME_DOME_TURN_TIME,   DEFAULT_AUTO_DOME_TURN_TIME);
        motorControllerBaudRate    = preferences.getInt(PREFERENCE_MOTOR_BAUD,            DEFAULT_MOTOR_BAUD);
        maestroBaudRate            = preferences.getInt(PREFERENCE_MAESTRO_BAUD,          DEFAULT_MAESTRO_BAUD);

        // ---- One-time migration of legacy random track keys (sm_rtmin/rtmax -> sm_rand_lo/hi) ----
        {
            int haveLo = preferences.getInt("sm_rand_lo", -1);
            int haveHi = preferences.getInt("sm_rand_hi", -1);
            if (haveLo < 0 || haveHi < 0) {
                int oldLo = preferences.getInt("sm_rtmin", -1);
                int oldHi = preferences.getInt("sm_rtmax", -1);
                if (oldLo > 0 && oldHi > 0) {
                    preferences.putInt("sm_rand_lo", oldLo);
                    preferences.putInt("sm_rand_hi", oldHi);
                    preferences.remove("sm_rtmin");
                    preferences.remove("sm_rtmax");
                }
            }
        }
    }
#endif

    // ---- NeoPixel persisted state ----
    neopixelCount   = preferences.getInt("neocount", 0);
    neopixelR       = preferences.getInt("neor", 0);
    neopixelG       = preferences.getInt("neog", 0);
    neopixelB       = preferences.getInt("neob", 0);
    neopixelEnabled = preferences.getBool("neoenable", false);
    if (neopixelCount > 0) {
        initNeoPixels(neopixelCount);
        refreshNeoPixels();
    }

    PrintReelTwoInfo(Serial, "Penumbra Shadow MD");
    DEBUG_PRINTLN("Bluetooth Library Started");

    // ---- PS3 Nav attach ----
    PS3NavFoot->attachOnInit(onInitPS3NavFoot);
    PS3NavDome->attachOnInit(onInitPS3NavDome);

    // ---- Motor serial / config ----
    MOTOR_SERIAL_INIT(motorControllerBaudRate);
    FootMotor->setTimeout(10);
    FootMotor->setDeadband(driveDeadBandRange);
    FootMotor->stop();
    DomeMotor->setTimeout(20);

    // ---- Maestro serials ----
    MAESTRO_SERIAL_INIT(maestroBaudRate);       // dome maestro
    BODY_MAESTRO_SERIAL_INIT(maestroBaudRate);  // body maestro

    SetupEvent::ready();

    if (Usb.Init() == -1)
    {
        DEBUG_PRINTLN("OSC did not start");
        while (1); // halt
    }

    // ---- Sound subsystem ----
#if defined(SHADOW_SOUND_PLAYER)
    // Unified prefs (aliases for clarity)
    const char* PREF_SOUND_MODULE             = PREFERENCE_SHADOWSOUND;
    const char* PREF_SOUND_VOLUME             = PREFERENCE_SHADOWSOUND_VOLUME;
    const char* PREF_SOUND_STARTUP            = PREFERENCE_SHADOWSOUND_STARTUP;
    const char* PREF_SOUND_RANDOM_MIN_MS      = PREFERENCE_SHADOWSOUND_RANDOM_MIN;
    const char* PREF_SOUND_RANDOM_MAX_MS      = PREFERENCE_SHADOWSOUND_RANDOM_MAX;
    const char* PREF_SOUND_RANDOM_ENABLED     = PREFERENCE_SHADOWSOUND_RANDOM;
    const char* PREF_SOUND_RANDOM_LO_TRACK    = "sm_rand_lo";
    const char* PREF_SOUND_RANDOM_HI_TRACK    = "sm_rand_hi";

    SDSound::Module soundPlayer =
        (SDSound::Module)preferences.getInt(PREF_SOUND_MODULE, SHADOW_SOUND_PLAYER);
    int soundStartup =
        preferences.getInt(PREF_SOUND_STARTUP, SHADOW_SOUND_STARTUP);

    // Baud per module (MP3 Trigger @ 38400, DFPlayer/DY-SV5W @ 9600, 0=disabled)
    uint32_t soundBaud = (soundPlayer == SDSound::kMP3Trigger) ? 38400 : 9600;

    SOUND_SERIAL.end();
    bool ok = false;
    if (soundBaud) {
        SOUND_SERIAL_INIT(soundBaud);
        ok = sShadowSound.begin(soundPlayer, SOUND_SERIAL, soundStartup);
    }

    if (!ok) {
        DEBUG_PRINTLN("FAILED TO INITALIZE SOUND MODULE");
    } else {
        // 1) Volume with guardrail (avoid silent boots)
        int volPref = preferences.getInt(PREF_SOUND_VOLUME, 700);
        if (volPref < 50) volPref = 700;
        sShadowSound.setVolume(volPref / 1000.0f);

        // 2) Random delay window
        uint32_t rmin = preferences.getInt(PREF_SOUND_RANDOM_MIN_MS, SHADOW_SOUND_RANDOM_MIN);
        uint32_t rmax = preferences.getInt(PREF_SOUND_RANDOM_MAX_MS, SHADOW_SOUND_RANDOM_MAX);
        sShadowSound.setRandomMin(rmin);
        sShadowSound.setRandomMax(rmax);

        // 3) Random track range (defaults 1..255)
        uint16_t rtlo = preferences.getInt(PREF_SOUND_RANDOM_LO_TRACK, 1);
        uint16_t rthi = preferences.getInt(PREF_SOUND_RANDOM_HI_TRACK, 255);
        if (rtlo < 1) rtlo = 1;
        if (rthi < rtlo) rthi = rtlo;
        if (rthi > 255) rthi = 255;
        sShadowSound.setRandomTracks(rtlo, rthi);

        // 4) Startup sound (if >0)
        if (soundStartup > 0) {
            delay(150); // let device settle for volume to latch
            sShadowSound.playSound(0, (uint8_t)soundStartup);
        }

        // 5) Start random if enabled
        if (preferences.getBool(PREFERENCE_SHADOWSOUND_RANDOM, SHADOW_SOUND_RANDOM)) {
            sShadowSound.startRandomInSeconds(13);
        }
    }
#endif
}

void sendMarcCommand(const char* cmd)
{
    SHADOW_VERBOSE("Sending MARC: \"%s\"\n", cmd)
    MAESTRO_SERIAL.print(cmd); MAESTRO_SERIAL.print("\r");

#if defined(SHADOW_SOUND_PLAYER)
    SHADOW_VERBOSE("Sound CMD: \"%s\"\n", cmd);
    // Pass-through to sound driver’s $-style parser.
    // (It safely ignores non-$ strings.)
    sShadowSound.handleCommand(cmd);
#endif
}

void sendBodyMarcCommand(const char* cmd)
{
    SHADOW_VERBOSE("Sending BODYMARC: \"%s\"\n", cmd)
    BODY_MAESTRO_SERIAL.print(cmd); BODY_MAESTRO_SERIAL.print("\r");
}

static bool handleSMPLAY(const char* s) {
  if (!s) return false;

  // Skip any leading whitespace after "#SMPLAY"
  while (*s && isspace((unsigned char)*s)) ++s;

  // If the number is glued (e.g., "3" from "#SMPLAY3"), or after a space ("#SMPLAY 3")
  // Move s to the first digit
  while (*s && !isdigit((unsigned char)*s)) ++s;

  if (!isdigit((unsigned char)*s)) {
    DEBUG_PRINTLN("SMPLAY: Missing track number");
    return false;
  }

  long n = strtol(s, nullptr, 10);
  if (n < 1) n = 1;
  if (n > 255) n = 255;

  DEBUG_PRINTLN("[SMPLAY] parsed track:");
  DEBUG_PRINTLN(n);

  sShadowSound.stopRandom();     // ensure random isn’t fighting you
  sShadowSound.playTrack((uint16_t)n);
  // If you want an echo like your current UI:
  // printf("Played track: %ld\n", n);
  return true;
}
// ------- SM console help -------
static void printSMHelp()
{
    printf("\nPenumbra / Maestro Console Commands\n");
    printf("-----------------------------------\n");
    printf("#SMHELP                    : Show this help.\n");
    printf("#SMDUMP                    : Print commands to restore current configuration.\n");
    printf("#SMZERO                    : Clear ALL preferences (factory reset) and reboot.\n");
    printf("#SMRESTART                 : Reboot the controller.\n");
    printf("#SMLIST                    : List all button actions.\n");
    printf("#SMDEL <TriggerName>       : Reset a trigger to its default action.\n");
    printf("#SMPLAY <TriggerName>      : Execute a trigger now.\n");
    printf("#SMSET <Trigger> <Action>  : Bind an action to a trigger.\n");
    printf("    Action formats:\n");
    printf("      DM<seq>              : Dome Maestro subroutine <seq>\n");
    printf("      BM<seq>              : Body Maestro subroutine <seq>\n");
    printf("      DM<seq>;S<num>       : Run <seq>, then play sound track <num>\n");
    printf("      DM<seq>;S R [a [b]]  : Run <seq>, then one-shot random in [a..b] (defaults to configured range)\n");
    printf("      S<num>               : Play sound track <num> only (flat index)\n");
    printf("      S R [a [b]]          : One-shot random sound from [a..b] (or configured range)\n");
    printf("      S<letter>            : Sound command $<LETTER> (e.g. Sf volume max)\n");
    printf("\n");
    printf("#SMSOUND0                  : Sound Disabled\n");
    printf("#SMSOUND1                  : Sound = MP3 Trigger (38400 baud)\n");
    printf("#SMSOUND2                  : Sound = DFPlayer Mini (9600 baud)\n");
    printf("#SMSOUND3                  : Sound = DY-SV5W (9600 baud)\n");
    printf("#SMVOLUME <0..1000>        : Sound volume (0=muted .. 1000=max)\n");
    printf("#SMSTARTUP <track|-1>      : Startup sound (track number) or -1 to disable\n");
    printf("#SMRAND0                   : Disable random sounds\n");
    printf("#SMRAND1                   : Enable random sounds (uses current random timing)\n");
    printf("#SMRANDMIN <ms>            : Random minimum delay (milliseconds)\n");
    printf("#SMRANDMAX <ms>            : Random maximum delay (milliseconds)\n");
    printf("\n");
    printf("#SMNORMALSPEED <0..127>    : Drive normal speed\n");
    printf("#SMMAXSPEED <0..127>       : Drive max speed (over-throttle)\n");
    printf("#SMTURNSPEED <0..127>      : Turn speed\n");
    printf("#SMDOMESPEED <0..127>      : Dome joystick speed\n");
    printf("#SMRAMPING <0..10>         : Accel/decel ramping level\n");
    printf("#SMFOOTDB <0..127>         : Foot stick deadband\n");
    printf("#SMDOMEDB <0..127>         : Dome stick deadband\n");
    printf("#SMDRIVEDB <0..127>        : Drive controller deadband\n");
    printf("#SMINVERT <0|1>            : Invert turn direction\n");
    printf("#SMAUTOSPEED <50..100>     : Auto dome speed (percent)\n");
    printf("#SMAUTOTIME <2000..8000>   : Auto dome turn time (ms)\n");
    printf("#SMMARCBAUD <baud>         : Maestro serial baud (needs reboot)\n");
    printf("#SMMOTORBAUD <baud>        : Motor controller baud (needs reboot)\n");
    printf("\n");
    printf("Examples:\n");
    printf(" - Bind button to Dome sequence:         #SMSET FTbtnUP_MD \"DM58\"\n");
    printf(" - Bind button to Body sequence:         #SMSET btnUP_MD    \"BM2\"\n");
    printf(" - Run sequence + track on press:        #SMSET FTbtnUP_MD \"DM58;S3\"\n");
    printf(" - Run sequence + random range:          #SMSET FTbtnUP_MD \"DM58;S R 10 25\"\n");
    printf(" - Sequences auto-queue: if DM/BM is running, next DM/BM will wait then run.\n");
    printf(" - Play only a track on press:           #SMSET btnRight_MD \"S 42\"\n");
    printf(" - Play a track immediately:             #SMPLAY 42\n");
    printf(" - One-shot random in range:             #SMPLAYRAND 10 25  (or bind: #SMSET btnX \"S R 10 25\")\n");
    printf(" - Persistent random range + enable:     #SMRANDTRACKS 10 25   (then #SMRAND1)\n");
    printf(" - List all trigger names to bind:       #SMLIST\n");
    printf("\n");
}

////////////////////////////////
// This function is called when settings have been changed and needs a reboot
void reboot()
{
    DEBUG_PRINTLN("Restarting...");
    preferences.end();
    delay(1000);
    ESP.restart();
}

// =======================================================================================
//           Main Program Loop - This is the recurring check loop for entire sketch
// =======================================================================================

// Route a single token that begins with '#'.
static void routeOne(char* line)
{
    char* cmd = line;

    // ---------- Command chain begins ----------
    if (matchCmd(cmd, "#SMHELP"))
    {
        printSMHelp();
    }
    if (matchCmd(cmd, "#SMZERO"))
    {
        preferences.clear();
        DEBUG_PRINT("Clearing preferences. ");
        reboot();
    }
    CMD("#SMRESTART")
    {
        reboot();
    }
    CMD("#SMLIST")
    {
        printf("Button Actions\n");
        printf("-----------------------------------\n");
        ShadowButtonAction::listActions();
    }
    CMD("#SMDEL")
    {
        String key(cmd); key.trim();
        ShadowButtonAction* btn = ShadowButtonAction::findAction(key);
        if (btn) {
            btn->reset();
            printf("Trigger: %s reset to default %s\n", btn->name().c_str(), btn->action().c_str());
        } else {
            printf("Trigger Not Found: %s\n", key.c_str());
        }
    }
    CMD("#SMVOLUME")
    {
        uint32_t val;
        if (parseUIntInRange(cmd, val, 0, 1000, "Usage: #SMVOLUME <0..1000>")) {
            preferences.putInt(PREFERENCE_SHADOWSOUND_VOLUME, (int)val);
            sShadowSound.setVolume(val / 1000.0f);
            printf("Sound Volume: %u (%.0f%%)\n", val, val / 10.0f);
        }
    }
    CMD("#SMSOUND")
    {
        long choice;
        if (!parseLongArg(cmd, choice)) {
            printf("Usage: #SMSOUND0 | #SMSOUND1 | #SMSOUND2 | #SMSOUND3\n");
        } else {
            SDSound::Module mod  = SDSound::fromChoice((int)choice);
            uint32_t        baud = SDSound::baudFor(mod);

            preferences.putInt(PREFERENCE_SHADOWSOUND, (int)mod);

            sShadowSound.end();
            SOUND_SERIAL.end();
            if (baud) {
                SOUND_SERIAL_INIT(baud);
                int startup = preferences.getInt(PREFERENCE_SHADOWSOUND_STARTUP, SHADOW_SOUND_STARTUP);
                if (!sShadowSound.begin(mod, SOUND_SERIAL, startup)) {
                    printf("FAILED TO INITIALIZE SOUND MODULE: %s (baud=%lu)\n",
                           SDSound::moduleName(mod), (unsigned long)baud);
                } else {
                    int volPref = preferences.getInt(PREFERENCE_SHADOWSOUND_VOLUME, 700);
                    if (volPref < 50) volPref = 700;
                    sShadowSound.setVolume(volPref / 1000.0f);
                    sShadowSound.setRandomMin(preferences.getInt(PREFERENCE_SHADOWSOUND_RANDOM_MIN, SHADOW_SOUND_RANDOM_MIN));
                    sShadowSound.setRandomMax(preferences.getInt(PREFERENCE_SHADOWSOUND_RANDOM_MAX, SHADOW_SOUND_RANDOM_MAX));
                    if (preferences.getBool(PREFERENCE_SHADOWSOUND_RANDOM, SHADOW_SOUND_RANDOM))
                        sShadowSound.startRandomInSeconds(13);
                }
            } else {
                printf("Sound Disabled.\n");
            }
            printf("Sound module set to: %s (%lu baud)\n",
                   SDSound::moduleName(mod), (unsigned long)baud);
        }
    }
    CMD("#SMCONFIG")
    {
        // Aliases (same as in setup)
        const char* PREF_SOUND_MODULE             = PREFERENCE_SHADOWSOUND;
        const char* PREF_SOUND_VOLUME             = PREFERENCE_SHADOWSOUND_VOLUME;
        const char* PREF_SOUND_STARTUP            = PREFERENCE_SHADOWSOUND_STARTUP;
        const char* PREF_SOUND_RANDOM_MIN_MS      = PREFERENCE_SHADOWSOUND_RANDOM_MIN;
        const char* PREF_SOUND_RANDOM_MAX_MS      = PREFERENCE_SHADOWSOUND_RANDOM_MAX;
        const char* PREF_SOUND_RANDOM_ENABLED     = PREFERENCE_SHADOWSOUND_RANDOM;

        SDSound::Module smod = (SDSound::Module)preferences.getInt(PREF_SOUND_MODULE, SHADOW_SOUND_PLAYER);
        int  vol   = preferences.getInt(PREF_SOUND_VOLUME,   SHADOW_SOUND_VOLUME);
        int  start = preferences.getInt(PREF_SOUND_STARTUP,  SHADOW_SOUND_STARTUP);
        bool rnd   = preferences.getBool(PREF_SOUND_RANDOM_ENABLED, SHADOW_SOUND_RANDOM);
        int  rmin  = preferences.getInt(PREF_SOUND_RANDOM_MIN_MS,   SHADOW_SOUND_RANDOM_MIN);
        int  rmax  = preferences.getInt(PREF_SOUND_RANDOM_MAX_MS,   SHADOW_SOUND_RANDOM_MAX);
        uint16_t rtlo, rthi; sShadowSound.getRandomTracks(rtlo, rthi);

        printf("Configuration\n");
        printf("-----------------------------------\n");
        // Sound
        printf("Sound Module:        %s   (#SMSOUND0/1/2/3)\n", SDSound::moduleName(smod));
        printf("Sound Volume:        %4d (#SMVOLUME)         [0..1000]\n", vol);
        printf("Startup Sound:       %4d (#SMSTARTUP)        [-1 disable | track]\n", start);
        printf("Random Enabled:      %4d (#SMRAND0/#SMRAND1) [0/1]\n", rnd ? 1 : 0);
        printf("Random Min Delay:    %4d (#SMRANDMIN)        [ms]\n", rmin);
        printf("Random Max Delay:    %4d (#SMRANDMAX)        [ms]\n", rmax);
        printf("Random Track Range:  %3u..%3u (#SMRANDTRACKS <min> <max>)\n", rtlo, rthi);

        // Drive / Dome
        printf("Drive Speed Normal:  %3d (#SMNORMALSPEED)    [0..127]\n", drivespeed1);
        printf("Drive Speed Max:     %3d (#SMMAXSPEED)       [0..127]\n", drivespeed2);
        printf("Turn Speed:          %3d (#SMTURNSPEED)      [0..127]\n", turnspeed);
        printf("Dome Speed:          %3d (#SMDOMESPEED)      [0..127]\n", domespeed);
        printf("Ramping:             %3d (#SMRAMPING)        [0..10]\n", ramping);
        printf("Foot Stick Deadband: %3d (#SMFOOTDB)         [0..127]\n", joystickFootDeadZoneRange);
        printf("Dome Stick Deadband: %3d (#SMDOMEDB)         [0..127]\n", joystickDomeDeadZoneRange);
        printf("Drive Deadband:      %3d (#SMDRIVEDB)        [0..127]\n", driveDeadBandRange);
        printf("Invert Turn:         %3d (#SMINVERT)         [0..1]\n",  invertTurnDirection);
        printf("Dome Auto Speed:     %3d (#SMAUTOSPEED)      [50..100]\n", domeAutoSpeed);
        printf("Dome Auto Time:     %4d (#SMAUTOTIME)       [2000..8000 ms]\n", time360DomeTurn);

        // Serial
        printf("Maestro Baud:     %6d (#SMMARCBAUD)\n",  maestroBaudRate);
        printf("Motor Baud:       %6d (#SMMOTORBAUD)\n", motorControllerBaudRate);

        // NeoPixels
        printf("NeoPixels Enabled:   %3d (#SMNEOON/#SMNEOOFF)\n", neopixelEnabled ? 1 : 0);
        printf("NeoPixel Count:      %3d (#SMNEOCOUNT)\n", neopixelCount);
        printf("NeoPixel Color:   R=%3d G=%3d B=%3d (#SMNEOCOLOR <r> <g> <b>)\n",
               neopixelR, neopixelG, neopixelB);
    }
    CMD("#SMDUMP")
    {
        // Emit a list of commands that can recreate the current configuration.
        printf("# ---- Config dump: paste to restore ----\n");
        const char* PREF_SOUND_MODULE         = PREFERENCE_SHADOWSOUND;
        const char* PREF_SOUND_VOLUME         = PREFERENCE_SHADOWSOUND_VOLUME;
        const char* PREF_SOUND_STARTUP        = PREFERENCE_SHADOWSOUND_STARTUP;
        const char* PREF_SOUND_RANDOM_MIN_MS  = PREFERENCE_SHADOWSOUND_RANDOM_MIN;
        const char* PREF_SOUND_RANDOM_MAX_MS  = PREFERENCE_SHADOWSOUND_RANDOM_MAX;
        const char* PREF_SOUND_RANDOM_ENABLED = PREFERENCE_SHADOWSOUND_RANDOM;

        SDSound::Module smod = (SDSound::Module)preferences.getInt(PREF_SOUND_MODULE, SHADOW_SOUND_PLAYER);
        int  vol   = preferences.getInt(PREF_SOUND_VOLUME,   SHADOW_SOUND_VOLUME);
        int  start = preferences.getInt(PREF_SOUND_STARTUP,  SHADOW_SOUND_STARTUP);
        bool rnd   = preferences.getBool(PREF_SOUND_RANDOM_ENABLED, SHADOW_SOUND_RANDOM);
        int  rmin  = preferences.getInt(PREF_SOUND_RANDOM_MIN_MS,   SHADOW_SOUND_RANDOM_MIN);
        int  rmax  = preferences.getInt(PREF_SOUND_RANDOM_MAX_MS,   SHADOW_SOUND_RANDOM_MAX);
        uint16_t rtlo, rthi; sShadowSound.getRandomTracks(rtlo, rthi);

        printf("#SMSOUND%u\n", (unsigned)smod);
        printf("#SMVOLUME %d\n", vol);
        printf("#SMSTARTUP %d\n", start);
        printf("#SMRANDMIN %d\n", rmin);
        printf("#SMRANDMAX %d\n", rmax);
        printf(rnd ? "#SMRAND1\n" : "#SMRAND0\n");
        printf("#SMRANDTRACKS %u %u\n", (unsigned)rtlo, (unsigned)rthi);

        printf("#SMNORMALSPEED %u\n", (unsigned)drivespeed1);
        printf("#SMMAXSPEED %u\n", (unsigned)drivespeed2);
        printf("#SMTURNSPEED %u\n", (unsigned)turnspeed);
        printf("#SMDOMESPEED %u\n", (unsigned)domespeed);
        printf("#SMRAMPING %u\n", (unsigned)ramping);
        printf("#SMFOOTDB %u\n", (unsigned)joystickFootDeadZoneRange);
        printf("#SMDOMEDB %u\n", (unsigned)joystickDomeDeadZoneRange);
        printf("#SMDRIVEDB %u\n", (unsigned)driveDeadBandRange);
        printf("#SMINVERT %u\n", invertTurnDirection ? 1u : 0u);
        printf("#SMAUTOSPEED %u\n", (unsigned)domeAutoSpeed);
        printf("#SMAUTOTIME %u\n", (unsigned)time360DomeTurn);

        printf("#SMMARCBAUD %d\n", maestroBaudRate);
        printf("#SMMOTORBAUD %d\n", motorControllerBaudRate);

        printf(neopixelEnabled ? "#SMNEOON\n" : "#SMNEOOFF\n");
        printf("#SMNEOCOUNT %d\n", neopixelCount);
        printf("#SMNEOCOLOR %u %u %u\n", (unsigned)neopixelR, (unsigned)neopixelG, (unsigned)neopixelB);

        // Dump triggers as SMSET commands
        ShadowButtonAction::dumpActions();
        printf("# ---- End of config dump ----\n");
    }
    CMD("#SMSTARTUP")
    {
        long v;
        if (parseLongArg(cmd, v)) {
            preferences.putInt(PREFERENCE_SHADOWSOUND_STARTUP, (int)v);
            printf("Startup Sound: %ld\n", v);
        } else {
            printf("Usage: #SMSTARTUP <track|-1>\n");
        }
    }
    CMD("#SMRANDMIN")
    {
        uint32_t v;
        if (parseUIntInRange(cmd, v, 0, 60000, "Usage: #SMRANDMIN <ms>")) {
            preferences.putInt(PREFERENCE_SHADOWSOUND_RANDOM_MIN, (int)v);
            sShadowSound.setRandomMin(v);
            printf("Random Min: %u\n", v);
        }
    }
    CMD("#SMRANDMAX")
    {
        uint32_t v;
        if (parseUIntInRange(cmd, v, 0, 60000, "Usage: #SMRANDMAX <ms>")) {
            preferences.putInt(PREFERENCE_SHADOWSOUND_RANDOM_MAX, (int)v);
            sShadowSound.setRandomMax(v);
            printf("Random Max: %u\n", v);
        }
    }
    CMD("#SMRAND0")
    {
        preferences.putInt(PREFERENCE_SHADOWSOUND_RANDOM, false);
        sShadowSound.stopRandom();
        printf("Random Disabled.\n");
    }
    CMD("#SMRAND1")
    {
        preferences.putBool(PREFERENCE_SHADOWSOUND_RANDOM, true);
        sShadowSound.startRandom();
        printf("Random Enabled.\n");
    }
    CMD("#SMNORMALSPEED")
    {
        uint32_t v;
        if (!parseUIntInRange(cmd, v, 0, 127, "Usage: #SMNORMALSPEED <0..127>")) {}
        else if (v == (uint32_t)drivespeed1) printf("Unchanged.\n");
        else { drivespeed1 = v; preferences.putInt(PREFERENCE_SPEED_NORMAL, drivespeed1); printf("Normal Speed Changed.\n"); }
    }
    CMD("#SMMAXSPEED")
    {
        uint32_t v;
        if (!parseUIntInRange(cmd, v, 0, 127, "Usage: #SMMAXSPEED <0..127>")) {}
        else if (v == (uint32_t)drivespeed2) printf("Unchanged.\n");
        else { drivespeed2 = v; preferences.putInt(PREFERENCE_SPEED_OVER_THROTTLE, drivespeed2); printf("Max Speed Changed.\n"); }
    }
    CMD("#SMTURNSPEED")
    {
        uint32_t v;
        if (!parseUIntInRange(cmd, v, 0, 127, "Usage: #SMTURNSPEED <0..127>")) {}
        else if (v == (uint32_t)turnspeed) printf("Unchanged.\n");
        else { turnspeed = v; preferences.putInt(PREFERENCE_TURN_SPEED, turnspeed); printf("Turn Speed Changed.\n"); }
    }
    CMD("#SMDOMESPEED")
    {
        uint32_t v;
        if (!parseUIntInRange(cmd, v, 0, 127, "Usage: #SMDOMESPEED <0..127>")) {}
        else if (v == (uint32_t)domespeed) printf("Unchanged.\n");
        else { domespeed = v; preferences.putInt(PREFERENCE_DOME_SPEED, v); printf("Dome Speed Changed.\n"); }
    }
    CMD("#SMRAMPING")
    {
        uint32_t v;
        if (!parseUIntInRange(cmd, v, 0, 10, "Usage: #SMRAMPING <0..10>")) {}
        else if (v == (uint32_t)ramping) printf("Unchanged.\n");
        else { ramping = v; preferences.putInt(PREFERENCE_RAMPING, ramping); printf("Ramping Changed.\n"); }
    }
    CMD("#SMFOOTDB")
    {
        uint32_t v;
        if (!parseUIntInRange(cmd, v, 0, 127, "Usage: #SMFOOTDB <0..127>")) {}
        else if (v == (uint32_t)joystickFootDeadZoneRange) printf("Unchanged.\n");
        else { joystickFootDeadZoneRange = v; preferences.putInt(PREFERENCE_FOOTSTICK_DEADBAND, joystickFootDeadZoneRange); printf("Foot Joystick Deadband Changed.\n"); }
    }
    CMD("#SMDOMEDB")
    {
        uint32_t v;
        if (!parseUIntInRange(cmd, v, 0, 127, "Usage: #SMDOMEDB <0..127>")) {}
        else if (v == (uint32_t)joystickDomeDeadZoneRange) printf("Unchanged.\n");
        else { joystickDomeDeadZoneRange = v; preferences.putInt(PREFERENCE_DOMESTICK_DEADBAND, joystickDomeDeadZoneRange); printf("Dome Joystick Deadband Changed.\n"); }
    }
    CMD("#SMDRIVEDB")
    {
        uint32_t v;
        if (!parseUIntInRange(cmd, v, 0, 127, "Usage: #SMDRIVEDB <0..127>")) {}
        else if (v == (uint32_t)driveDeadBandRange) printf("Unchanged.\n");
        else { driveDeadBandRange = v; preferences.putInt(PREFERENCE_DRIVE_DEADBAND, driveDeadBandRange); printf("Drive Controller Deadband Changed.\n"); }
    }
    CMD("#SMINVERT")
    {
        long v;
        if (!parseLongArg(cmd, v)) { printf("Usage: #SMINVERT <0|1>\n"); }
        else if ((bool)v == invertTurnDirection) printf("Unchanged.\n");
        else { invertTurnDirection = (bool)v; preferences.putInt(PREFERENCE_INVERT_TURN_DIRECTION, invertTurnDirection);
            printf("Invert Turn Direction %s.\n", invertTurnDirection ? "Enabled" : "Disabled"); }
    }
    CMD("#SMAUTOSPEED")
    {
        uint32_t v;
        if (!parseUIntInRange(cmd, v, 50, 100, "Usage: #SMAUTOSPEED <50..100>")) {}
        else if (v == (uint32_t)domeAutoSpeed) printf("Unchanged.\n");
        else { domeAutoSpeed = v; preferences.putInt(PREFERENCE_DOME_AUTO_SPEED, domeAutoSpeed); printf("Auto Dome Speed Changed.\n"); }
    }
    CMD("#SMAUTOTIME")
    {
        uint32_t v;
        if (!parseUIntInRange(cmd, v, 0, 8000, "Usage: #SMAUTOTIME <0..8000>")) {}
        else if (v == (uint32_t)time360DomeTurn) printf("Unchanged.\n");
        else { time360DomeTurn = v; preferences.putInt(PREFERENCE_DOME_DOME_TURN_TIME, time360DomeTurn); printf("Auto Dome Turn Time Changed.\n"); }
    }
    CMD("#SMMOTORBAUD")
    {
        long v;
        if (parseLongArg(cmd, v)) {
            if (v == motorControllerBaudRate) printf("Unchanged.\n");
            else { motorControllerBaudRate = (int)v; preferences.putInt(PREFERENCE_MOTOR_BAUD, motorControllerBaudRate);
                printf("Motor Controller Serial Baud Rate Changed. Needs Reboot.\n"); }
        } else printf("Usage: #SMMOTORBAUD <baud>\n");
    }
    CMD("#SMMARCBAUD")
    {
        long v;
        if (parseLongArg(cmd, v)) {
            if (v == maestroBaudRate) printf("Unchanged.\n");
            else { maestroBaudRate = (int)v; preferences.putInt(PREFERENCE_MAESTRO_BAUD, maestroBaudRate);
                printf("Maestro Serial Baud Rate Changed. Needs Reboot.\n"); }
        } else printf("Usage: #SMMARCBAUD <baud>\n");
    }
    CMD("#SMPLAY")
    {
        // ---- debug: show what we actually received at this point
        static unsigned long smplay_seq = 0;
        ++smplay_seq;
        printf("[SMPLAY %lu] raw after token: \"%s\"\n", smplay_seq, cmd);

        // Accept "#SMPLAY 3" or "#SMPLAY3"
        _skip_ws(cmd);

        if (isdigit((unsigned char)*cmd)) {
            char* endp = nullptr;
            long v = strtol(cmd, &endp, 10);
            if (endp == cmd) {
                printf("Usage: #SMPLAY <1..255 | TriggerName>\n");
                return;
            }
            if (v < 1)   v = 1;
            if (v > 255) v = 255;

            const uint16_t track = (uint16_t)v;

            // ---- debug: show exactly what we parsed
            printf("[SMPLAY %lu] parsed: %u\n", smplay_seq, (unsigned)track);

            // Don’t let random override an explicit play
            sShadowSound.stopRandom();

            // Play exactly what we parsed
            sShadowSound.playTrack(track);

            // Echo exactly what we sent (single source of truth)
            printf("Played track: %u\n", (unsigned)track);

            // Advance past the number so the same chars aren't re-consumed
            cmd = endp;
            _skip_ws(cmd);
            return;
        }

        // Otherwise, treat argument as a trigger name
        String key(cmd);
        key.trim();
        ShadowButtonAction* btn = ShadowButtonAction::findAction(key);
        if (btn) {
            btn->trigger();
            printf("Triggered: %s\n", key.c_str());
        } else {
            printf("Unknown: %s\n", key.c_str());
        }
        return;
    }
    CMD("#SMSET")
    {
        _skip_ws(cmd);
        char* keyp = cmd;
        char* valp = strchr(cmd, ' ');
        if (valp) {
            *valp++ = '\0';
            String key(keyp); key.trim();
            ShadowButtonAction* btn = ShadowButtonAction::findAction(key);
            if (btn) {
                String action(valp); action.trim();
                btn->setAction(action);
                printf("Trigger: %s set to %s\n", key.c_str(), action.c_str());
            } else {
                printf("Trigger Not Found: %s\n", key.c_str());
            }
        } else {
            printf("Usage: #SMSET <TriggerName> <Action>\n");
        }
    }
    CMD("#SMNEOON")
    {
        neopixelEnabled = true;
        preferences.putBool("neoenable", true);
        refreshNeoPixels();
        printf("NeoPixels Enabled.\n");
    }
    CMD("#SMNEOOFF")
    {
        neopixelEnabled = false;
        preferences.putBool("neoenable", false);
        refreshNeoPixels();
        printf("NeoPixels Disabled.\n");
    }
    CMD("#SMNEOCOUNT")
    {
        uint32_t v;
        if (parseUIntInRange(cmd, v, 0, 300, "Usage: #SMNEOCOUNT <count>")) {
            neopixelCount = v;
            preferences.putInt("neocount", neopixelCount);
            initNeoPixels(neopixelCount);
            refreshNeoPixels();
            printf("NeoPixel count set to %u\n", v);
        }
    }
    CMD("#SMNEOCOLOR")
    {
        long r, g, b;
        if (!parseLongArg(cmd, r)) { printf("Usage: #SMNEOCOLOR <R> <G> <B>\n"); }
        else if (!parseLongArg(cmd, g)) { printf("Usage: #SMNEOCOLOR <R> <G> <B>\n"); }
        else if (!parseLongArg(cmd, b)) { printf("Usage: #SMNEOCOLOR <R> <G> <B>\n"); }
        else {
            if (r < 0) r = 0; if (r > 255) r = 255;
            if (g < 0) g = 0; if (g > 255) g = 255;
            if (b < 0) b = 0; if (b > 255) b = 255;

            neopixelR = r; neopixelG = g; neopixelB = b;
            preferences.putInt("neor", neopixelR);
            preferences.putInt("neog", neopixelG);
            preferences.putInt("neob", neopixelB);
            refreshNeoPixels();
            printf("NeoPixel color set to R=%ld G=%ld B=%ld\n", r, g, b);
        }
    }
    // ---- One-shot random play in a range ----
    CMD("#SMPLAYRAND")
    {
        long lo, hi;
        if (!parseLongArg(cmd, lo)) {
            printf("Usage: #SMPLAYRAND <min> [max]\n");
        } else {
            if (!parseLongArg(cmd, hi)) hi = lo;
            if (lo < 1) lo = 1;
            if (hi < 1) hi = 1;
            if (lo > 255) lo = 255;
            if (hi > 255) hi = 255;
            if (hi < lo) { long t = lo; lo = hi; hi = t; }
            sShadowSound.playRandomTrack((uint16_t)lo, (uint16_t)hi);
            printf("Random track %ld–%ld triggered\n", lo, hi);
        }
    }
    // ---- Persistent random track range (single definition) ----
    CMD("#SMRANDTRACKS")
    {
        long lo, hi;
        if (!parseLongArg(cmd, lo) || !parseLongArg(cmd, hi)) {
            printf("Usage: #SMRANDTRACKS <min> <max>\n");
        } else {
            if (lo < 1) lo = 1;
            if (hi < 1) hi = 1;
            if (lo > 255) lo = 255;
            if (hi > 255) hi = 255;
            if (hi < lo) { long t = lo; lo = hi; hi = t; }
            sShadowSound.setRandomTracks((uint16_t)lo, (uint16_t)hi);
            preferences.putInt("sm_rand_lo", (int)lo);
            preferences.putInt("sm_rand_hi", (int)hi);
            printf("Random track range set to %ld–%ld\n", lo, hi);
        }
    }
    // ---------- End of command chain ----------
    else
    {
        printf("Unknown command: %s\n", line);
        printf("Valid commands:\n");
        printSMHelp();
    }
}

void loop()
{
    //LOOP through functions from highest to lowest priority.
    if (!readUSB())
        return;
    
    footMotorDrive();
    domeDrive();
    marcDuinoDome();
    marcDuinoFoot();
    toggleSettings();    
#if defined(SHADOW_SOUND_PLAYER)
    sShadowSound.idle();
#endif

    // Service any queued Maestro DM/BM subroutine requests
    pumpMaestroQueues();

    // If dome automation is enabled - Call function
    if (domeAutomation && time360DomeTurn > 1999 && time360DomeTurn < 8001 && domeAutoSpeed > 49 && domeAutoSpeed < 101)  
    {
       autoDome(); 
    }

   // New serial input handling (CRLF-safe, tokenized)
   while (Serial.available())
   {
       int ch = Serial.read();
       if (ch < 0) break;

       // Handle CR/LF with CRLF swallow
        if (ch == '\r' || ch == '\n')
        {
            if (ch == '\n' && sLastWasCR)
            {
                sLastWasCR = false;
                continue; // swallow LF of CRLF pair
            }
            sLastWasCR = (ch == '\r');

            // Null-terminate and route tokens on this line
            sBuffer[sPos] = '\0';

            if (sPos > 0)
            {
                char* p = sBuffer;
                while (*p)
                {
                    while (*p && *p != '#') ++p;       // find next token
                    if (!*p) break;
                    char* start = p;
                    ++p;                                 // skip '#'
                    while (*p && *p != '#') ++p;        // advance to next '#'
                    char saved = *p;
                    *p = '\0';                          // isolate token

                    // trim spaces
                    while (*start && isspace((unsigned char)*start)) ++start;
                    char* end = start + strlen(start);
                    while (end > start && isspace((unsigned char)end[-1])) --end;
                    *end = '\0';

                    if (*start)
                    {
                        routeOne(start);
                    }

                    *p = saved;                         // restore
                }
            }

            // hard reset line buffer
            sPos = 0;
            sBuffer[0] = '\0';
            continue;
        }

        // Normal character append with overflow guard
        sLastWasCR = false;
        if (sPos + 1 < SizeOfArray(sBuffer))
        {
            sBuffer[sPos++] = (char)ch;
            sBuffer[sPos] = '\0';
        }
        else
        {
            // overflow: reset to avoid concatenation garbage
            sPos = 0;
            sBuffer[0] = '\0';
        }
   }

}

// =======================================================================================
//           footDrive Motor Control Section
// =======================================================================================

bool ps3FootMotorDrive(PS3BT* myPS3 = PS3NavFoot)
{
    int stickSpeed = 0;
    int turnnum = 0;
  
    if (isPS3NavigatonInitialized)
    {    
         // Additional fault control.  Do NOT send additional commands to Sabertooth if no controllers have initialized.
        if (!isStickEnabled)
        {
        #ifdef SHADOW_VERBOSE
            if (abs(myPS3->getAnalogHat(LeftHatY)-128) > joystickFootDeadZoneRange)
            {
                SHADOW_VERBOSE("Drive Stick is disabled\n")
            }
        #endif

            if (!isFootMotorStopped)
            {
                FootMotor->stop();
                isFootMotorStopped = true;
                footDriveSpeed = 0;
              
                SHADOW_VERBOSE("\n***Foot Motor STOPPED***\n")
            }
            return false;
        }
        else if (!myPS3->PS3NavigationConnected)
        {
            if (!isFootMotorStopped)
            {
                FootMotor->stop();
                isFootMotorStopped = true;
                footDriveSpeed = 0;

                SHADOW_VERBOSE("\n***Foot Motor STOPPED***\n")
            }
          
            return false;
        }
        else if (myPS3->getButtonPress(L2) || myPS3->getButtonPress(L1))
        {
            if (!isFootMotorStopped)
            {
                FootMotor->stop();
                isFootMotorStopped = true;
                footDriveSpeed = 0;

                SHADOW_VERBOSE("\n***Foot Motor STOPPED***\n")              
            }
            return false;
        }
        else
        {
            int joystickPosition = myPS3->getAnalogHat(LeftHatY);
          
            if (overSpeedSelected) //Over throttle is selected
            {
                stickSpeed = (map(joystickPosition, 0, 255, -drivespeed2, drivespeed2));   
            }
            else 
            {
                stickSpeed = (map(joystickPosition, 0, 255, -drivespeed1, drivespeed1));
            }

            if ( abs(joystickPosition-128) < joystickFootDeadZoneRange)
            {
                // This is RAMP DOWN code when stick is now at ZERO but prior FootSpeed > 20
                if (abs(footDriveSpeed) > 50)
                {   
                    if (footDriveSpeed > 0)
                    {
                        footDriveSpeed -= 3;
                    }
                    else
                    {
                        footDriveSpeed += 3;
                    }
                    SHADOW_VERBOSE("ZERO FAST RAMP: footSpeed: %d\nStick Speed: %d\n", footDriveSpeed, stickSpeed)
                }
                else if (abs(footDriveSpeed) > 20)
                {   
                    if (footDriveSpeed > 0)
                    {
                        footDriveSpeed -= 2;
                    }
                    else
                    {
                        footDriveSpeed += 2;
                    }  
                    SHADOW_VERBOSE("ZERO MID RAMP: footSpeed: %d\nStick Speed: %d\n", footDriveSpeed, stickSpeed)
                }
                else
                {        
                    footDriveSpeed = 0;
                }
            }
            else 
            {
                isFootMotorStopped = false;
                if (footDriveSpeed < stickSpeed)
                {
                    if ((stickSpeed-footDriveSpeed)>(ramping+1))
                    {
                        footDriveSpeed+=ramping;
                        SHADOW_VERBOSE("RAMPING UP: footSpeed: %d\nStick Speed: %d\n", footDriveSpeed, stickSpeed)
                    }
                    else
                    {
                        footDriveSpeed = stickSpeed;
                    }
                }
                else if (footDriveSpeed > stickSpeed)
                {
                    if ((footDriveSpeed-stickSpeed)>(ramping+1))
                    {
                        footDriveSpeed-=ramping;
                        SHADOW_VERBOSE("RAMPING DOWN: footSpeed: %d\nStick Speed: %d\n", footDriveSpeed, stickSpeed)
                    }
                    else
                    {
                        footDriveSpeed = stickSpeed;  
                    }
                }
                else
                {
                    footDriveSpeed = stickSpeed;  
                }
            }
            turnnum = (myPS3->getAnalogHat(LeftHatX));

            //TODO:  Is there a better algorithm here?  
            if ( abs(footDriveSpeed) > 50)
                turnnum = (map(myPS3->getAnalogHat(LeftHatX), 54, 200, -(turnspeed/4), (turnspeed/4)));
            else if (turnnum <= 200 && turnnum >= 54)
                turnnum = (map(myPS3->getAnalogHat(LeftHatX), 54, 200, -(turnspeed/3), (turnspeed/3)));
            else if (turnnum > 200)
                turnnum = (map(myPS3->getAnalogHat(LeftHatX), 201, 255, turnspeed/3, turnspeed));
            else if (turnnum < 54)
                turnnum = (map(myPS3->getAnalogHat(LeftHatX), 0, 53, -turnspeed, -(turnspeed/3)));
              
            if (abs(turnnum) > 5)
            {
                isFootMotorStopped = false;   
            }

            currentMillis = millis();
          
            if ((currentMillis - previousFootMillis) > serialLatency)
            {
                if (footDriveSpeed != 0 || abs(turnnum) > 5)
                {
                    SHADOW_VERBOSE("Motor: FootSpeed: %d\nTurnnum: %d\nTime of command: %lu\n", footDriveSpeed, turnnum, millis())              
                    FootMotor->turn(turnnum * (invertTurnDirection ? 1 : -1));
                    FootMotor->drive(footDriveSpeed);
                }
                else
                {    
                    if (!isFootMotorStopped)
                    {
                        FootMotor->stop();
                        isFootMotorStopped = true;
                        footDriveSpeed = 0;
                      
                        SHADOW_VERBOSE("\n***Foot Motor STOPPED***\n")
                    }
                }
              
                // The Sabertooth won't act on mixed mode packet serial commands until
                // it has received power levels for BOTH throttle and turning, since it
                // mixes the two together to get diff-drive power levels for both motors.
              
                previousFootMillis = currentMillis;
                return true; //we sent a foot command   
            }
        }
    }
    return false;
}

void footMotorDrive()
{
    //Flood control prevention
    if ((millis() - previousFootMillis) < serialLatency)
        return;
  
    if (PS3NavFoot->PS3NavigationConnected)
        ps3FootMotorDrive(PS3NavFoot);
}  


// =======================================================================================
//           domeDrive Motor Control Section
// =======================================================================================

int ps3DomeDrive(PS3BT* myPS3 = PS3NavDome)
{
    int domeRotationSpeed = 0;
    int joystickPosition = myPS3->getAnalogHat(LeftHatX);
        
    domeRotationSpeed = (map(joystickPosition, 0, 255, -domespeed, domespeed));
    if ( abs(joystickPosition-128) < joystickDomeDeadZoneRange ) 
       domeRotationSpeed = 0;
          
    if (domeRotationSpeed != 0 && domeAutomation == true)  // Turn off dome automation if manually moved
    {   
        domeAutomation = false; 
        domeStatus = 0;
        domeTargetPosition = 0; 
        
        SHADOW_VERBOSE("Dome Automation OFF\n")
    }    
    return domeRotationSpeed;
}

void rotateDome(int domeRotationSpeed, String mesg)
{
    //Constantly sending commands to the SyRen (Dome) is causing foot motor delay.
    //Lets reduce that chatter by trying 3 things:
    // 1.) Eliminate a constant stream of "don't spin" messages (isDomeMotorStopped flag)
    // 2.) Add a delay between commands sent to the SyRen (previousDomeMillis timer)
    // 3.) Switch to real UART on the MEGA (Likely the *CORE* issue and solution)
    // 4.) Reduce the timout of the SyRen - just better for safety!
    
    currentMillis = millis();
    if ((!isDomeMotorStopped || domeRotationSpeed != 0) &&
        ((currentMillis - previousDomeMillis) > (2*serialLatency)))
    {
        if (domeRotationSpeed != 0)
        {
            isDomeMotorStopped = false;
            SHADOW_VERBOSE("Dome rotation speed: %d\n", domeRotationSpeed)        
            DomeMotor->motor(domeRotationSpeed);
        }
        else
        {
            isDomeMotorStopped = true; 
            SHADOW_VERBOSE("\n***Dome motor is STOPPED***\n")
            
            DomeMotor->stop();
        }
        previousDomeMillis = currentMillis;      
    }
}

void domeDrive()
{
    //Flood control prevention
    //This is intentionally set to double the rate of the Dome Motor Latency
    if ((millis() - previousDomeMillis) < (2*serialLatency))
        return;
  
    int domeRotationSpeed = 0;
    int ps3NavControlSpeed = 0;
    if (PS3NavDome->PS3NavigationConnected) 
    {
        ps3NavControlSpeed = ps3DomeDrive(PS3NavDome);
        domeRotationSpeed = ps3NavControlSpeed; 

        rotateDome(domeRotationSpeed,"Controller Move");
    }
    else if (PS3NavFoot->PS3NavigationConnected && PS3NavFoot->getButtonPress(L2))
    {
        ps3NavControlSpeed = ps3DomeDrive(PS3NavFoot);
        domeRotationSpeed = ps3NavControlSpeed;

        rotateDome(domeRotationSpeed,"Controller Move");    
    }
    else if (!isDomeMotorStopped)
    {
        DomeMotor->stop();
        isDomeMotorStopped = true;
    }  
}  

// =======================================================================================
//                               Toggle Control Section
// =======================================================================================

void ps3ToggleSettings(PS3BT* myPS3 = PS3NavFoot)
{
    // enable / disable drive stick
    if (myPS3->getButtonPress(PS) && myPS3->getButtonClick(CROSS))
    {
        SHADOW_DEBUG("Disabling the DriveStick\n")
        SHADOW_DEBUG("Stopping Motors\n")

        FootMotor->stop();
        isFootMotorStopped = true;
        isStickEnabled = false;
        footDriveSpeed = 0;
    }
    
    if(myPS3->getButtonPress(PS) && myPS3->getButtonClick(CIRCLE))
    {
        SHADOW_DEBUG("Enabling the DriveStick\n");
        isStickEnabled = true;
    }
    
    // Enable and Disable Overspeed
    if (myPS3->getButtonPress(L3) && myPS3->getButtonPress(L1) && isStickEnabled)
    {
        if ((millis() - previousSpeedToggleMillis) > 1000)
        {
            speedToggleButtonCounter = 0;
            previousSpeedToggleMillis = millis();
        } 

        speedToggleButtonCounter += 1;       
        if (speedToggleButtonCounter == 1)
        {
            if (!overSpeedSelected)
            {
                overSpeedSelected = true;
                SHADOW_VERBOSE("Over Speed is now: ON\n");
            }
            else
            {      
                overSpeedSelected = false;
                SHADOW_VERBOSE("Over Speed is now: OFF\n")
            }
        }
    }
   
    // Enable Disable Dome Automation
    if(myPS3->getButtonPress(L2) && myPS3->getButtonClick(CROSS))
    {
        domeAutomation = false;
        domeStatus = 0;
        domeTargetPosition = 0;
        DomeMotor->stop();
        isDomeMotorStopped = true;
        
        SHADOW_DEBUG("Dome Automation OFF\n")
    } 

    if(myPS3->getButtonPress(L2) && myPS3->getButtonClick(CIRCLE))
    {
        domeAutomation = true;

        SHADOW_DEBUG("Dome Automation On\n")
    } 
}

void toggleSettings()
{
    if (PS3NavFoot->PS3NavigationConnected)
        ps3ToggleSettings(PS3NavFoot);
}  

// ====================================================================================================================
// This function determines if MarcDuino buttons were selected and calls main processing function for FOOT controller
// ====================================================================================================================
void marcDuinoFoot()
{
    if (PS3NavFoot->PS3NavigationConnected && (PS3NavFoot->getButtonPress(UP) || PS3NavFoot->getButtonPress(DOWN) || PS3NavFoot->getButtonPress(LEFT) || PS3NavFoot->getButtonPress(RIGHT)))
    {
        if ((millis() - previousMarcDuinoMillis) > 1000)
        {
            marcDuinoButtonCounter = 0;
            previousMarcDuinoMillis = millis();
        }
       marcDuinoButtonCounter += 1;
    }
    else
    {
        return;    
    }
   
    //------------------------------------ 
    // Send triggers for the base buttons 
    //------------------------------------
    if (PS3NavFoot->getButtonPress(UP) && !PS3NavFoot->getButtonPress(CROSS) && !PS3NavFoot->getButtonPress(CIRCLE) && !PS3NavFoot->getButtonPress(L1) && !PS3NavFoot->getButtonPress(PS) && marcDuinoButtonCounter == 1)
    {
        if (PS3NavDome->PS3NavigationConnected && (PS3NavDome->getButtonPress(CROSS) || PS3NavDome->getButtonPress(CIRCLE) || PS3NavDome->getButtonPress(PS)))
        {
            // Skip this section
        }
        else
        {
            btnUP_MD.trigger();
            return;
        }
    }
    
    if (PS3NavFoot->getButtonPress(DOWN) && !PS3NavFoot->getButtonPress(CROSS) && !PS3NavFoot->getButtonPress(CIRCLE) && !PS3NavFoot->getButtonPress(L1) && !PS3NavFoot->getButtonPress(PS) && marcDuinoButtonCounter == 1)
    {
        if (PS3NavDome->PS3NavigationConnected && (PS3NavDome->getButtonPress(CROSS) || PS3NavDome->getButtonPress(CIRCLE) || PS3NavDome->getButtonPress(PS)))
        {
            // Skip this section
        }
        else
        {     
            btnDown_MD.trigger();
            return;
        }
    }
    
    if (PS3NavFoot->getButtonPress(LEFT) && !PS3NavFoot->getButtonPress(CROSS) && !PS3NavFoot->getButtonPress(CIRCLE) && !PS3NavFoot->getButtonPress(L1) && !PS3NavFoot->getButtonPress(PS) && marcDuinoButtonCounter == 1)
    {
        if (PS3NavDome->PS3NavigationConnected && (PS3NavDome->getButtonPress(CROSS) || PS3NavDome->getButtonPress(CIRCLE) || PS3NavDome->getButtonPress(PS)))
        {
            // Skip this section
        }
        else
        {           
            btnLeft_MD.trigger();
            return;
        }
    }

    if (PS3NavFoot->getButtonPress(RIGHT) && !PS3NavFoot->getButtonPress(CROSS) && !PS3NavFoot->getButtonPress(CIRCLE) && !PS3NavFoot->getButtonPress(L1) && !PS3NavFoot->getButtonPress(PS) && marcDuinoButtonCounter == 1)
    {
        if (PS3NavDome->PS3NavigationConnected && (PS3NavDome->getButtonPress(CROSS) || PS3NavDome->getButtonPress(CIRCLE) || PS3NavDome->getButtonPress(PS)))
        {
            // Skip this section
        }
        else
        {     
            btnRight_MD.trigger();;
            return;
        }
    }
    
    //------------------------------------ 
    // Send triggers for the CROSS + base buttons 
    //------------------------------------
    if (((!PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(UP) && PS3NavFoot->getButtonPress(CROSS)) || (PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(UP) && PS3NavDome->getButtonPress(CROSS))) && marcDuinoButtonCounter == 1)
    {
        btnUP_CROSS_MD.trigger();
        return;        
    }
    
    if (((!PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(DOWN) && PS3NavFoot->getButtonPress(CROSS)) || (PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(DOWN) && PS3NavDome->getButtonPress(CROSS))) && marcDuinoButtonCounter == 1)
    {      
        btnDown_CROSS_MD.trigger();
        return;
    }
    
    if (((!PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(LEFT) && PS3NavFoot->getButtonPress(CROSS)) || (PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(LEFT) && PS3NavDome->getButtonPress(CROSS))) && marcDuinoButtonCounter == 1)
    {
        btnLeft_CROSS_MD.trigger();;
        return;
    }

    if (((!PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(RIGHT) && PS3NavFoot->getButtonPress(CROSS)) || (PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(RIGHT) && PS3NavDome->getButtonPress(CROSS))) && marcDuinoButtonCounter == 1)
    {
        btnRight_CROSS_MD.trigger();
        return;
    }

    //------------------------------------ 
    // Send triggers for the CIRCLE + base buttons 
    //------------------------------------
    if (((!PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(UP) && PS3NavFoot->getButtonPress(CIRCLE)) || (PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(UP) && PS3NavDome->getButtonPress(CIRCLE))) && marcDuinoButtonCounter == 1)
    {
        btnUP_CIRCLE_MD.trigger();
        return;
    }
    
    if (((!PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(DOWN) && PS3NavFoot->getButtonPress(CIRCLE)) || (PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(DOWN) && PS3NavDome->getButtonPress(CIRCLE))) && marcDuinoButtonCounter == 1)
    {
        btnDown_CIRCLE_MD.trigger();
        return;
    }
    
    if (((!PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(LEFT) && PS3NavFoot->getButtonPress(CIRCLE)) || (PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(LEFT) && PS3NavDome->getButtonPress(CIRCLE))) && marcDuinoButtonCounter == 1)
    {
        btnLeft_CIRCLE_MD.trigger();
        return;
    }

    if (((!PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(RIGHT) && PS3NavFoot->getButtonPress(CIRCLE)) || (PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(RIGHT) && PS3NavDome->getButtonPress(CIRCLE))) && marcDuinoButtonCounter == 1)
    {
        btnRight_CIRCLE_MD.trigger();
        return;
    }
    
    //------------------------------------ 
    // Send triggers for the L1 + base buttons 
    //------------------------------------
    if (PS3NavFoot->getButtonPress(UP) && PS3NavFoot->getButtonPress(L1) && marcDuinoButtonCounter == 1)
    {
        btnUP_L1_MD.trigger();
        return;
    }
    
    if (PS3NavFoot->getButtonPress(DOWN) && PS3NavFoot->getButtonPress(L1) && marcDuinoButtonCounter == 1)
    {
        btnDown_L1_MD.trigger();
        return;
    }
    
    if (PS3NavFoot->getButtonPress(LEFT) && PS3NavFoot->getButtonPress(L1) && marcDuinoButtonCounter == 1)
    {
        btnLeft_L1_MD.trigger();
        return;
    }

    if (PS3NavFoot->getButtonPress(RIGHT) && PS3NavFoot->getButtonPress(L1) && marcDuinoButtonCounter == 1)
    {
        btnRight_L1_MD.trigger();
        return;
    }
    
    //------------------------------------ 
    // Send triggers for the PS + base buttons 
    //------------------------------------
    if (((!PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(UP) && PS3NavFoot->getButtonPress(PS)) || (PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(UP) && PS3NavDome->getButtonPress(PS))) && marcDuinoButtonCounter == 1)
    {
        btnUP_PS_MD.trigger();
        return;
    }
    
    if (((!PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(DOWN) && PS3NavFoot->getButtonPress(PS)) || (PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(DOWN) && PS3NavDome->getButtonPress(PS))) && marcDuinoButtonCounter == 1)
    {
        btnDown_PS_MD.trigger();
        return;
    }
    
    if (((!PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(LEFT) && PS3NavFoot->getButtonPress(PS)) || (PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(LEFT) && PS3NavDome->getButtonPress(PS))) && marcDuinoButtonCounter == 1)
    {
        btnLeft_PS_MD.trigger();
        return;
    }

    if (((!PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(RIGHT) && PS3NavFoot->getButtonPress(PS)) || (PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(RIGHT) && PS3NavDome->getButtonPress(PS))) && marcDuinoButtonCounter == 1)
    {
        btnRight_PS_MD.trigger();
        return;
    }
}

// ===================================================================================================================
// This function determines if MarcDuino buttons were selected and calls main processing function for DOME Controller
// ===================================================================================================================
void marcDuinoDome()
{
    if (PS3NavDome->PS3NavigationConnected && (PS3NavDome->getButtonPress(UP) || PS3NavDome->getButtonPress(DOWN) || PS3NavDome->getButtonPress(LEFT) || PS3NavDome->getButtonPress(RIGHT)))
    {
        if ((millis() - previousMarcDuinoMillis) > 1000)
        {
            marcDuinoButtonCounter = 0;
            previousMarcDuinoMillis = millis();
        }
        marcDuinoButtonCounter += 1;
    }
    else
    {
        return;    
    }

    //------------------------------------ 
    // Send triggers for the base buttons 
    //------------------------------------
    if (PS3NavDome->getButtonPress(UP) && !PS3NavDome->getButtonPress(CROSS) && !PS3NavDome->getButtonPress(CIRCLE) && !PS3NavDome->getButtonPress(L1) && !PS3NavDome->getButtonPress(PS) && !PS3NavFoot->getButtonPress(CROSS) && !PS3NavFoot->getButtonPress(CIRCLE) && !PS3NavFoot->getButtonPress(PS) && marcDuinoButtonCounter == 1)
    {
        FTbtnUP_MD.trigger();
        return;
    }
    
    if (PS3NavDome->getButtonPress(DOWN) && !PS3NavDome->getButtonPress(CROSS) && !PS3NavDome->getButtonPress(CIRCLE) && !PS3NavDome->getButtonPress(L1) && !PS3NavDome->getButtonPress(PS) && !PS3NavFoot->getButtonPress(CROSS) && !PS3NavFoot->getButtonPress(CIRCLE) && !PS3NavFoot->getButtonPress(PS) && marcDuinoButtonCounter == 1)
    {
        FTbtnDown_MD.trigger();
        return;      
    }
    
    if (PS3NavDome->getButtonPress(LEFT) && !PS3NavDome->getButtonPress(CROSS) && !PS3NavDome->getButtonPress(CIRCLE) && !PS3NavDome->getButtonPress(L1) && !PS3NavDome->getButtonPress(PS) && !PS3NavFoot->getButtonPress(CROSS) && !PS3NavFoot->getButtonPress(CIRCLE) && !PS3NavFoot->getButtonPress(PS) && marcDuinoButtonCounter == 1)
    {
        FTbtnLeft_MD.trigger();
        return;
    }

    if (PS3NavDome->getButtonPress(RIGHT) && !PS3NavDome->getButtonPress(CROSS) && !PS3NavDome->getButtonPress(CIRCLE) && !PS3NavDome->getButtonPress(L1) && !PS3NavDome->getButtonPress(PS) && !PS3NavFoot->getButtonPress(CROSS) && !PS3NavFoot->getButtonPress(CIRCLE) && !PS3NavFoot->getButtonPress(PS) && marcDuinoButtonCounter == 1)
    {
        FTbtnRight_MD.trigger();
        return;
    }
    
    //------------------------------------ 
    // Send triggers for the CROSS + base buttons 
    //------------------------------------
    if (PS3NavDome->getButtonPress(UP) && PS3NavFoot->getButtonPress(CROSS) && marcDuinoButtonCounter == 1)
    {
        FTbtnUP_CROSS_MD.trigger();
        return;
    }
    
    if (PS3NavDome->getButtonPress(DOWN) && PS3NavFoot->getButtonPress(CROSS) && marcDuinoButtonCounter == 1)
    {
        FTbtnDown_CROSS_MD.trigger();
        return;
    }
    
    if (PS3NavDome->getButtonPress(LEFT) && PS3NavFoot->getButtonPress(CROSS) && marcDuinoButtonCounter == 1)
    {
        FTbtnLeft_CROSS_MD.trigger();
        return;
    }

    if (PS3NavDome->getButtonPress(RIGHT) && PS3NavFoot->getButtonPress(CROSS) && marcDuinoButtonCounter == 1)
    {
        FTbtnRight_CROSS_MD.trigger();
        return;
    }

    //------------------------------------ 
    // Send triggers for the CIRCLE + base buttons 
    //------------------------------------
    if (PS3NavDome->getButtonPress(UP) && PS3NavFoot->getButtonPress(CIRCLE) && marcDuinoButtonCounter == 1)
    {
        FTbtnUP_CIRCLE_MD.trigger();
        return;
    }
    
    if (PS3NavDome->getButtonPress(DOWN) && PS3NavFoot->getButtonPress(CIRCLE) && marcDuinoButtonCounter == 1)
    {
        FTbtnDown_CIRCLE_MD.trigger();
        return;
    }
    
    if (PS3NavDome->getButtonPress(LEFT) && PS3NavFoot->getButtonPress(CIRCLE) && marcDuinoButtonCounter == 1)
    {
        FTbtnLeft_CIRCLE_MD.trigger();
        return;
    }

    if (PS3NavDome->getButtonPress(RIGHT) && PS3NavFoot->getButtonPress(CIRCLE) && marcDuinoButtonCounter == 1)
    {
        FTbtnRight_CIRCLE_MD.trigger();
        return;
    }
    
    //------------------------------------ 
    // Send triggers for the L1 + base buttons 
    //------------------------------------
    if (PS3NavDome->getButtonPress(UP) && PS3NavDome->getButtonPress(L1) && marcDuinoButtonCounter == 1)
    {
        FTbtnUP_L1_MD.trigger();
        return;
    }
    
    if (PS3NavDome->getButtonPress(DOWN) && PS3NavDome->getButtonPress(L1) && marcDuinoButtonCounter == 1)
    {
        FTbtnDown_L1_MD.trigger();
        return;
    }
    
    if (PS3NavDome->getButtonPress(LEFT) && PS3NavDome->getButtonPress(L1) && marcDuinoButtonCounter == 1)
    {
        FTbtnLeft_L1_MD.trigger();
        return;
    }

    if (PS3NavDome->getButtonPress(RIGHT) && PS3NavDome->getButtonPress(L1) && marcDuinoButtonCounter == 1)
    {
        FTbtnRight_L1_MD.trigger();
        return;
    }
    
    //------------------------------------ 
    // Send triggers for the PS + base buttons 
    //------------------------------------
    if (PS3NavDome->getButtonPress(UP) && PS3NavFoot->getButtonPress(PS) && marcDuinoButtonCounter == 1)
    {
        FTbtnUP_PS_MD.trigger();
        return;
    }
    
    if (PS3NavDome->getButtonPress(DOWN) && PS3NavFoot->getButtonPress(PS) && marcDuinoButtonCounter == 1)
    {
        FTbtnDown_PS_MD.trigger();
        return;
    }
    
    if (PS3NavDome->getButtonPress(LEFT) && PS3NavFoot->getButtonPress(PS) && marcDuinoButtonCounter == 1)
    {
        FTbtnLeft_PS_MD.trigger();
        return;
    }

    if (PS3NavDome->getButtonPress(RIGHT) && PS3NavFoot->getButtonPress(PS) && marcDuinoButtonCounter == 1)
    {
        FTbtnRight_PS_MD.trigger();
        return;
    }
}

// =======================================================================================
//                             Dome Automation Function
//
//    Features toggles 'on' via L2 + CIRCLE.  'off' via L2 + CROSS.  Default is 'off'.
//
//    This routines randomly turns the dome motor in both directions.  It assumes the 
//    dome is in the 'home' position when the auto dome feature is toggled on.  From
//    there it turns the dome in a random direction.  Stops for a random length of 
//    of time.  Then returns the dome to the home position.  This randomly repeats.
//
//    It is driven off the user variable - time360DomeTurn.  This records how long
//    it takes the dome to do a 360 degree turn at the given auto dome speed.  Tweaking
//    this parameter to be close provides the best results.
//
//    Activating the dome controller manually immediately cancels the auto dome feature
//    or you can toggle the feature off by pressing L2 + CROSS.
// =======================================================================================
void autoDome()
{
    long rndNum;
    int domeSpeed;
    
    if (domeStatus == 0)  // Dome is currently stopped - prepare for a future turn
    { 
        if (domeTargetPosition == 0)  // Dome is currently in the home position - prepare to turn away
        {
            domeStartTurnTime = millis() + (random(3, 10) * 1000);

            rndNum = random(5,354);

            domeTargetPosition = rndNum;  // set the target position to a random degree of a 360 circle - shaving off the first and last 5 degrees

            if (domeTargetPosition < 180)  // Turn the dome in the positive direction
            {
                domeTurnDirection = 1;
                domeStopTurnTime = domeStartTurnTime + ((domeTargetPosition / 360) * time360DomeTurn);              
            }
            else  // Turn the dome in the negative direction
            {
                domeTurnDirection = -1;
                domeStopTurnTime = domeStartTurnTime + (((360 - domeTargetPosition) / 360) * time360DomeTurn);              
            }
        }
        else  // Dome is not in the home position - send it back to home
        {
            domeStartTurnTime = millis() + (random(3, 10) * 1000);   
            if (domeTargetPosition < 180)
            {
                domeTurnDirection = -1;
                domeStopTurnTime = domeStartTurnTime + ((domeTargetPosition / 360) * time360DomeTurn);
            }
            else
            {
                domeTurnDirection = 1;
                domeStopTurnTime = domeStartTurnTime + (((360 - domeTargetPosition) / 360) * time360DomeTurn);
            }
            domeTargetPosition = 0;
        }
        domeStatus = 1;  // Set dome status to preparing for a future turn

        SHADOW_DEBUG("Dome Automation: Initial Turn Set\nCurrent Time: %lu\nNext Start Time: %lu\nNext Stop Time: %lu\nDome Target Position: %f\n",
            millis(), domeStartTurnTime, domeStopTurnTime, domeTargetPosition);
    }    

    if (domeStatus == 1)  // Dome is prepared for a future move - start the turn when ready
    {
        if (domeStartTurnTime < millis())
        {
            domeStatus = 2; 
            SHADOW_DEBUG("Dome Automation: Ready To Start Turn\n")
        }
    }
    
    if (domeStatus == 2) // Dome is now actively turning until it reaches its stop time
    {      
        if (domeStopTurnTime > millis())
        {
            domeSpeed = domeAutoSpeed * domeTurnDirection;
            DomeMotor->motor(domeSpeed);

            SHADOW_DEBUG("Turning Now!!\n")
        }
        else  // turn completed - stop the motor
        {
            domeStatus = 0;
            DomeMotor->stop();

            SHADOW_DEBUG("STOP TURN!!\n")
        }      
    }
}

// =======================================================================================
//           PS3 Controller Device Mgt Functions
// =======================================================================================

void onInitPS3NavFoot()
{
    String btAddress = getLastConnectedBtMAC();
    PS3NavFoot->setLedOn(LED1);
    isPS3NavigatonInitialized = true;
    badPS3Data = 0;

    SHADOW_DEBUG("\nBT Address of Last connected Device when FOOT PS3 Connected: %s\n", btAddress.c_str());
    
    if (btAddress == PS3ControllerFootMac || btAddress == PS3ControllerBackupFootMac)
    {
        SHADOW_DEBUG("\nWe have our FOOT controller connected.\n")
          
        mainControllerConnected = true;
        WaitingforReconnect = true;
    }
#ifdef USE_PREFERENCES
    else if (PS3ControllerFootMac[0] == 'X')
    {
        SHADOW_DEBUG("\nAssigning %s as FOOT controller.\n", btAddress.c_str());
          
        preferences.putString(PREFERENCE_PS3_FOOT_MAC, btAddress);
        PS3ControllerFootMac = btAddress;
        mainControllerConnected = true;
        WaitingforReconnect = true;
    }
#endif
    else
    {
        // Prevent connection from anything but the MAIN controllers          
        SHADOW_DEBUG("\nWe have an invalid controller trying to connect as tha FOOT controller, it will be dropped.\n")

        FootMotor->stop();
        DomeMotor->stop();
        isFootMotorStopped = true;
        footDriveSpeed = 0;
        PS3NavFoot->setLedOff(LED1);
        PS3NavFoot->disconnect();
    
        isPS3NavigatonInitialized = false;
        mainControllerConnected = false;        
    } 
}

void onInitPS3NavDome()
{
    String btAddress = getLastConnectedBtMAC();
    PS3NavDome->setLedOn(LED1);
    isSecondaryPS3NavigatonInitialized = true;
    badPS3Data = 0;
    
    if (btAddress == PS3ControllerDomeMAC || btAddress == PS3ControllerBackupDomeMAC)
    {
        SHADOW_DEBUG("\nWe have our DOME controller connected.\n")
          
        domeControllerConnected = true;
        WaitingforReconnectDome = true;
    }
#ifdef USE_PREFERENCES
    else if (PS3ControllerDomeMAC[0] == 'X')
    {
        SHADOW_DEBUG("\nAssigning %s as DOME controller.\n", btAddress.c_str());
          
        preferences.putString(PREFERENCE_PS3_DOME_MAC, btAddress);
        PS3ControllerDomeMAC = btAddress;

        domeControllerConnected = true;
        WaitingforReconnectDome = true;
    }
#endif
    else
    {
        // Prevent connection from anything but the DOME controllers          
        SHADOW_DEBUG("\nWe have an invalid controller trying to connect as the DOME controller, it will be dropped.\n")

        FootMotor->stop();
        DomeMotor->stop();
        isFootMotorStopped = true;
        footDriveSpeed = 0;
        PS3NavDome->setLedOff(LED1);
        PS3NavDome->disconnect();
    
        isSecondaryPS3NavigatonInitialized = false;
        domeControllerConnected = false;        
    } 
}

String getLastConnectedBtMAC()
{
    char buffer[20];
    uint8_t* addr = Btd.disc_bdaddr;
    snprintf(buffer, sizeof(buffer), "%02X:%02X:%02X:%02X:%02X:%02X",
        addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
    return buffer;
}

bool criticalFaultDetect()
{
    if (PS3NavFoot->PS3NavigationConnected || PS3NavFoot->PS3Connected)
    {
        currentTime = millis();
        lastMsgTime = PS3NavFoot->getLastMessageTime();
        msgLagTime = currentTime - lastMsgTime;            
        
        if (WaitingforReconnect)
        {
            if (msgLagTime < 200)
            {             
                WaitingforReconnect = false; 
            }
            lastMsgTime = currentTime;            
        } 

        if (currentTime >= lastMsgTime)
        {
            msgLagTime = currentTime - lastMsgTime;
        }
        else
        {
            msgLagTime = 0;
        }
        
        if (msgLagTime > 300 && !isFootMotorStopped)
        {
            SHADOW_DEBUG("It has been 300ms since we heard from the PS3 Foot Controller\n")
            SHADOW_DEBUG("Shutting down motors, and watching for a new PS3 Foot message\n")
            FootMotor->stop();
            isFootMotorStopped = true;
            footDriveSpeed = 0;
        }
        
        if ( msgLagTime > 10000 )
        {
            SHADOW_DEBUG("It has been 10s since we heard from the PS3 Foot Controller\nmsgLagTime:%u  lastMsgTime:%u  millis: %lu\n",
                          msgLagTime, lastMsgTime, millis())
            SHADOW_DEBUG("Disconnecting the Foot controller\n")
            FootMotor->stop();
            isFootMotorStopped = true;
            footDriveSpeed = 0;
            PS3NavFoot->disconnect();
            WaitingforReconnect = true;
            return true;
        }

        //Check PS3 Signal Data
        if(!PS3NavFoot->getStatus(Plugged) && !PS3NavFoot->getStatus(Unplugged))
        {
            //We don't have good data from the controller.
            //Wait 15ms if no second controller - 100ms if some controller connected, Update USB, and try again
            if (PS3NavDome->PS3NavigationConnected)
            {
                  delay(100);     
            } else
            {
                  delay(15);
            }
            
            // Usb.Task();   
            lastMsgTime = PS3NavFoot->getLastMessageTime();
            
            if(!PS3NavFoot->getStatus(Plugged) && !PS3NavFoot->getStatus(Unplugged))
            {
                badPS3Data++;
                SHADOW_DEBUG("\n**Invalid data from PS3 FOOT Controller. - Resetting Data**\n")
                return true;
            }
        }
        else if (badPS3Data > 0)
        {

            badPS3Data = 0;
        }
        
        if ( badPS3Data > 10 )
        {
            SHADOW_DEBUG("Too much bad data coming from the PS3 FOOT Controller\n")
            SHADOW_DEBUG("Disconnecting the controller and stop motors.\n")

            FootMotor->stop();
            isFootMotorStopped = true;
            footDriveSpeed = 0;
            PS3NavFoot->disconnect();
            WaitingforReconnect = true;
            return true;
        }
    }
    else if (!isFootMotorStopped)
    {
        SHADOW_DEBUG("No foot controller was found\n")
        SHADOW_DEBUG("Shuting down motors and watching for a new PS3 foot message\n")

        FootMotor->stop();
        isFootMotorStopped = true;
        footDriveSpeed = 0;
        WaitingforReconnect = true;
        return true;
    }
    
    return false;
}

bool criticalFaultDetectDome()
{
    if (PS3NavDome->PS3NavigationConnected || PS3NavDome->PS3Connected)
    {
        currentTime = millis();
        lastMsgTime = PS3NavDome->getLastMessageTime();
        msgLagTime = currentTime - lastMsgTime;            
        
        if (WaitingforReconnectDome)
        {
            if (msgLagTime < 200)
            {
                WaitingforReconnectDome = false; 
            }            
            lastMsgTime = currentTime;
        }
        
        if (currentTime >= lastMsgTime)
        {
            msgLagTime = currentTime - lastMsgTime;
              
        }
        else
        {
            msgLagTime = 0;
        }
        
        if ( msgLagTime > 10000 )
        {
            SHADOW_DEBUG("It has been 10s since we heard from the PS3 Dome Controller\nmsgLagTime:%u  lastMsgTime:%u  millis: %lu\n",
                          msgLagTime, lastMsgTime, millis())
            SHADOW_DEBUG("Disconnecting the Foot controller\n")
            
            DomeMotor->stop();
            PS3NavDome->disconnect();
            WaitingforReconnectDome = true;
            return true;
        }

        //Check PS3 Signal Data
        if (!PS3NavDome->getStatus(Plugged) && !PS3NavDome->getStatus(Unplugged))
        {
            // We don't have good data from the controller.
            //Wait 100ms, Update USB, and try again
            delay(100);
            
            // Usb.Task();
            lastMsgTime = PS3NavDome->getLastMessageTime();
            
            if(!PS3NavDome->getStatus(Plugged) && !PS3NavDome->getStatus(Unplugged))
            {
                badPS3DataDome++;
                SHADOW_DEBUG("\n**Invalid data from PS3 Dome Controller. - Resetting Data**\n")
                return true;
            }
        }
        else if (badPS3DataDome > 0)
        {
            badPS3DataDome = 0;
        }
        
        if (badPS3DataDome > 10)
        {
            SHADOW_DEBUG("Too much bad data coming from the PS3 DOME Controller\n")
            SHADOW_DEBUG("Disconnecting the controller and stop motors.\n")

            DomeMotor->stop();
            PS3NavDome->disconnect();
            WaitingforReconnectDome = true;
            return true;
        }
    }
    return false;
}

// =======================================================================================
//           USB Read Function - Supports Main Program Loop
// =======================================================================================

bool readUSB()
{     
    Usb.Task();

    //The more devices we have connected to the USB or BlueTooth, the more often Usb.Task need to be called to eliminate latency.
    if (PS3NavFoot->PS3NavigationConnected) 
    {
        if (criticalFaultDetect())
        {
            //We have a fault condition that we want to ensure that we do NOT process any controller data!!!
            return false;
        }   
    }
    else if (!isFootMotorStopped)
    {
        SHADOW_DEBUG("No foot controller was found\n")
        SHADOW_DEBUG("Shuting down motors, and watching for a new PS3 foot message\n")

        FootMotor->stop();
        isFootMotorStopped = true;
        footDriveSpeed = 0;
        WaitingforReconnect = true;
    }
    
    if (PS3NavDome->PS3NavigationConnected) 
    {
        if (criticalFaultDetectDome())
        {
           //We have a fault condition that we want to ensure that we do NOT process any controller data!!!
           return false;
        }
    }    
    return true;
}
