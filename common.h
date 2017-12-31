#ifndef _COMMON_H
#define _COMMON_H

#include <HardwareSerial.h>

#include <LCD.h>

// Pins
#define BUT_PIN1 A0
#define BUT_PIN2 A1

#define RED_PIN 11
#define GRN_PIN 5
#define BLU_PIN 3

#define SCRN_SCLK_PIN 7
#define SCRN_MOSI_PIN 8
#define SCRN_DC_PIN 9
#define SCRN_RST_PIN 10
#define SCRN_SCE_PIN 12
#define SCRN_LED_PIN 6 // PWM

// Settings
#define nALARMS 1

// I2C address for DAC
#define MCP4726_ADDR 0x62

// Helper macros
#define histequal(a, b, hist) (a > b-hist && a < b+hist) // Is a close enough to b?

// types
enum mode {mode_INVALID, mode_NORMAL, mode_SETTIME, mode_SETALARM, mode_DEMO, mode_NMODES};
enum but {but_NONE, but_LEFT, but_RIGHT, but_UP, but_DOWN, but_SELECT, but_LEFT_LONG, but_RIGHT_LONG, but_UP_LONG, but_DOWN_LONG, but_SELECT_LONG, but_LEFT_RPT, but_RIGHT_RPT, but_UP_RPT, but_DOWN_RPT, but_SELECT_RPT};
struct alarm
{
	uint8_t hour;
	uint8_t min;
	uint8_t enDayEn; // bit 7 is "alarm in use", bits 6-0 are a weekday bit field for sunday-saturday
};

// sounds
extern const char sound[16000];

// externs
extern LCD lcd;
extern char const lookup[];
extern struct alarm alarms[nALARMS];

// strings
extern const char* emptyLine;

// Debugging stuff - smaller, faster code if DEBUG is not defined
#define DEBUG // for now

#ifdef DEBUG
#define _FILENAME_ "alarmClock.ino"
#define _XSTRINGIFY(x) #x
#define STRINGIFY(x) _XSTRINGIFY(x)
#define log(msg) \
					do \
					{ \
						Serial.print(_FILENAME_ ":" STRINGIFY(__LINE__) " in `"); \
						Serial.print(__PRETTY_FUNCTION__); \
						Serial.println("`, " msg); \
					} \
					while(0)
					
#define logobj(obj) \
					do \
					{ \
						Serial.println(obj); \
					} \
					while(0)
					
#define logwobj(msg, obj) \
					do \
					{ \
						log(msg); \
						logobj(obj); \
					} \
					while(0)
#else
#define log(msg)
#define logobj(obj)
#define logwobj(msg, obj)
#endif // DEBUG

#endif // _COMMON_H
