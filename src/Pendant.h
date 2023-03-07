// Main Pendant declarations for FluidNC Pendant
//
#include <Arduino.h>

// Config file contains Wi-Fi connection details and FluidNC hostname/port
#include "Config.h"

#include <Wifi.h>
#include <FluidNC_WebSocket.h>

#include <Wire.h>
#include <RotaryEncoder.h>
#include <AceButton.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include <esp_wifi.h>
#include <TFT_eSPI.h>      // Hardware-specific library
using namespace ace_button;

#define EEPROM_SIZE 1

TFT_eSPI tft = TFT_eSPI(); // Invoke custom library

RotaryEncoder *encoderA = nullptr;

// I/O pin definitions
#define PIN_REM1 GPIO_NUM_15
#define PIN_REM2 GPIO_NUM_13
#define PIN_JSX GPIO_NUM_32
#define PIN_JSY GPIO_NUM_33
#define PIN_REMS GPIO_NUM_2
#define PIN_YELLOW GPIO_NUM_25
#define PIN_GREEN GPIO_NUM_26
#define PIN_RED GPIO_NUM_27
#define PIN_JS_VCC GPIO_NUM_17
#define PIN_BATT 34
#define ADC_EN  14

#define TFT_LEFT    1
#define TFT_CENTER  2
#define TFT_RIGHT   3
#define TFT_TRANSP  true
#define TFT_OBLQ  false
#define TFT_LIGHTBLUE 0x86FF
#define TFT_LIGHTRED 0xF9E7
#define TFT_DARKBLUE 0x0018
#define TFT_OVERYELLOW 0xFF35
#define TFT_MEDIUMGREEN 0x04A2
#define TFT_DARKRED 0xA800
#define TFT_MEDIUMBLUE 0x3D5F
#define TFT_DARKORANGE 0xFAC0

#define MAX_JOGSTR 50
#define MAX_STATUSSTR 50
#define POWEROFF_TIME 300
#define LINEHEIGHT_DEF  22

enum pendantstate {
  Menu,
  MenuConfirm,
  Pendant,
  RotateChoice,
  MenuSpindleSpeed,
  MenuBrightness
};

enum menuOptions {
  poweroff,
  spindlespeed,
  spindleonoff,
  steps,
  quit,
  home,
  setwxy0,
  setwz0,
  probe,
  gawxy0,
  unlock,
  resetgrbl,
  brightness
};

enum rotorDestin  {
  RotXaxis,
  RotYaxis,
  RotZaxis,
  RotSpindlespeed,
  RotJogspeed,
  RotOvfeed,
  RotOvspeed,
  RotOvrapid
};

enum joystickDestin {
  jsXYaxis,
  jsZaxis
};

volatile pendantstate pState = Pendant;
volatile menuOptions menuState = quit;
volatile rotorDestin rState = RotJogspeed;
volatile joystickDestin jState = jsXYaxis;

const int maxMenu = 12;
const int jogSpeedMax = 5000; //dit moet gelijk lopen met de settings in config.yaml
static char mStateStr[][11] { "? ?", "ALARM", "IDLE", "JOG", "HOME", "CHECK", "RUN", "CYCLE", "HOLD", "SAFE", "SLEEP" };
const int powerOffTime = 60;
volatile bool powerOffNow = false;

String name = "FluidNC";

FluidNC_WS myCNC = FluidNC_WS();

#define DEBUG_SERIAL Serial

char statusStr[MAX_STATUSSTR];
volatile char g_sbuf[128];
volatile byte menuChoice = 0, backLight = 1;
volatile unsigned long lastJogCmdXYZ = 0, lastStepCmdXYZ = 0;
unsigned long tExec = 1000;
unsigned long statusUpdateInterval = 1000;
const int updateInterval = 198;
volatile float distX = 0, distY = 0, distZ = 0;
volatile int battVolt;
unsigned long sinceStart, lastGrblState;
int rdX = 0, rdY = 0, rdZ = 0, multXYZ = 0;
int calibrateX, calibrateY;
//float mX, mY, mZ;  //Machine positions
//float wX, wY, wZ;  //Work positions
RTC_DATA_ATTR int jogSpeed = 1000; //Overleeft een herstart vanuit sleep
int jogSpeedDisp = 0, setSpindleSpeed = 3000, spindleDisp = -1;
//int reportedSpindleSpeed = 0;
//int ovSpeed = 0, ovFeed = 0, ovRapid = 0;
//bool spindleOn = false;
volatile bool smallSteps = false;
volatile int stepXYZ = 0;
AceButton buttonREMS, buttonGREEN, buttonYELLOW, buttonRED;

// Predeclare functions
void ConnectWiFi(void);
void otaLoop(void);
IRAM_ATTR void checkPositionA(void);
void forceEndJog(void);
void waitEndJog(void);
int readJSX(void);
int readJSY(void);
int checkJoystick(void);
void checkRotateA(void);
void checkBattery(void);
void printAxisCoords(float m, float w, byte row);
void printCircleBlue (byte x, byte y);
void tftPrintAllButtons();
void tftPrintButtonOrange(String txt);
void tftPrintButtonGreen(String txt);
void tftPrintButtonRed(String txt);
void tftPrintButton(int bc, int tc, byte col, byte row, byte len, String txt);
void tftUpdate(bool force);
void tftPrint(int align, bool transp, int fg, int bg, byte col, byte row, byte len, String txt);
void tftPrintBig(int bg, int fg, byte col, byte row, byte len, String txt);
void tftPrintColor(int color, byte col, byte row, String txt);
void tftPrintSimple(byte col, byte row, String txt);
void activeTimer(bool reset);
void getGrblState(bool full);

#include "SleepTimer.h"
#include "Menu.h"
#include "ButtonHandlers.h"
#include "Joystick.h"
#include "Rotator.h"
#include "TFT_Draw.h"
#include "Setup.h"
#include "Misc.h"
