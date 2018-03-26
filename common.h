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
#define nALARMS 3
#define ALARM_LED_DURATION (30*60) // 30min
#define ALARM_BIRD_DURATION (15*60) // 15min
#define ALARM_DURATION (ALARM_LED_DURATION)
#define RED_MAX 255L
#define GRN_MAX 127
#define BLU_MAX 63

// I2C address for DAC
#define MCP4726_ADDR 0x62

// Helper macros
#define histequal(a, b, hist) (a >= b-hist && a <= b+hist) // Is a close enough to b?
// #define min(x, y) ((x < y) ? x : y)
// #define max(x, y) ((x > y) ? x : y)

// Config Addresses
#define CONFIG_ALARMS_ADDR 128

// types
enum mode {mode_INVALID, mode_NORMAL, mode_SETTIME, mode_SETALARM, mode_DEMO, mode_NMODES};
enum but {but_NONE, but_LEFT, but_RIGHT, but_UP, but_DOWN, but_SELECT, but_LEFT_LONG, but_RIGHT_LONG, but_UP_LONG, but_DOWN_LONG, but_SELECT_LONG, but_LEFT_RPT, but_RIGHT_RPT, but_UP_RPT, but_DOWN_RPT, but_SELECT_RPT};
#pragma pack(push, 1)
struct alarm
{
	uint8_t hour;
	uint8_t min;
	uint8_t enDayEn; // bit 7 is "alarm in use", bits 6-0 are a weekday bit field for sunday-saturday
};
#define ALARMSTOR_STRUCTVER 1
struct alarmStor
{
	uint8_t structVer;
	struct alarm val;
	unsigned long crc32;
};
#pragma pack(pop)
struct soundInfo
{
	int len;
	const char* sound;
};

// sounds
extern const char sound[16000];
extern const char alarmSound[2];
#define nBirdSounds 6
extern const char bird1[];
extern const char bird2[];
extern const char bird3[];
extern const char bird4[];
extern const char bird5[];
extern const char bird6[];
extern const struct soundInfo birdSounds[nBirdSounds];

// externs
extern LCD lcd;
extern char const lookup[];
extern struct alarm alarms[nALARMS];

// strings
extern const char* emptyLine;

// Debugging stuff - smaller, faster code if DEBUG is not defined
#define DEBUG // for now

#ifdef DEBUG
extern char sercmd[20];
extern int cmdlen;
extern char cmdval;
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
					
#define logobj(obj, ...) \
					do \
					{ \
						Serial.println(obj, ##__VA_ARGS__); \
					} \
					while(0)
					
#define logwobj(msg, obj, ...) \
					do \
					{ \
						log(msg); \
						logobj(obj, ##__VA_ARGS__); \
					} \
					while(0)
#else
#define log(msg)
#define logobj(obj, ...)
#define logwobj(msg, obj, ...)
#endif // DEBUG

#endif // _COMMON_H
