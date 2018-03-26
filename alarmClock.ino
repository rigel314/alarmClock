// TODO: add function that prints a help icon on the screen that takes an array of enums, which are indexes into a string table.
// TODO: add 2min timeout for modes that goes back to normal mode, so the alarm still goes off.
// TODO: reverify 8kHz sound, since I removed a mask

#include <TimerOne.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <Wire.h>
#include <avr/pgmspace.h>
#include <EEPROM.h>

#include "common.h"
#include "modes.h"
#include "util.h"

LCD lcd(SCRN_SCLK_PIN, SCRN_MOSI_PIN, SCRN_DC_PIN, SCRN_RST_PIN, SCRN_SCE_PIN);

const char sound[] PROGMEM =
#include "bird.h" // This has the data and a semicolon

const struct soundInfo birdSounds[] = {{2304, bird1}, {1504, bird2}, {1088, bird3}, {800, bird4}, {800, bird5}, {1024, bird6}};

const char lookup[] = "0123456789ABCDEF";
const char* emptyLine = "              ";

#ifdef DEBUG
char sercmd[20];
int cmdlen = 0;
char cmdval = 0;
#endif

struct alarm alarms[nALARMS] = {{0,0,0}};

volatile unsigned char gotTimer1 = 0;
volatile uint8_t secCtr = 0;
volatile uint8_t longPressCtr = 0;
volatile uint8_t realButPressed = 0;
volatile uint8_t longPressDetected = 0;
volatile uint8_t rptPressDetected = 0;
// Called via timer interrupt
void timer1inter()
{
	if(realButPressed && longPressCtr < 15)
		longPressCtr++;
	else if (!realButPressed)
		longPressCtr = 0;
	if(longPressCtr == 15)
	{
		if(longPressDetected)
			rptPressDetected = 1;
		longPressDetected = 1;
	}

	secCtr++;
	if(secCtr == 10)
	{
		gotTimer1 = 1;
		secCtr = 0;
		// logobj("inter");
	}
}

void setup()
{
	Serial.begin(115200); // For debug (owns pins 0 and 1)
	
	// TWBR = ((F_CPU / 400000L) - 16) / 2; // Set I2C frequency to 400kHz
	Wire.setClock(400000L);
	
	Timer1.initialize(100000); // (10Hz)
	Timer1.attachInterrupt(timer1inter);
	
	lcd.initialize();
	lcd.sendCommands("\x21\xAC\x20"); // Set contrast
	
	// set time sync to read from the RTC every 5min. (which is actually sync()'d in now() if it detects elapsed time >= 300s)
	setSyncProvider(RTC.get);
	// setSyncInterval(10);
	
	// Initial screen
	tmElements_t tm;
	breakTime(now(), tm);
	char timestr[] = "now - 15:37   ";
	timestr[6] = lookup[tm.Hour/10];
	timestr[7] = lookup[tm.Hour%10];
	timestr[9] = lookup[tm.Minute/10];
	timestr[10] = lookup[tm.Minute%10];
	lcd.sendString(0, 0, timestr);
	lcd.sendString(5, 0, "mode: NORMAL");
	
	// Set pin directions
	pinMode(BUT_PIN1, INPUT);
	pinMode(BUT_PIN2, INPUT);
	
	pinMode(RED_PIN, OUTPUT);
	pinMode(GRN_PIN, OUTPUT);
	pinMode(BLU_PIN, OUTPUT);
	
	// read alarm time and other settings(backlight intensity?) from EEPROM/RTCram
	struct alarmStor tmpAlrm;
	for (int i = 0; i < nALARMS; i++)
	{
		int addr = CONFIG_ALARMS_ADDR + i*sizeof(struct alarmStor);
		EEPROM.get(addr, tmpAlrm);
		unsigned long crcCalc = crc32((char*)&tmpAlrm, sizeof(struct alarmStor) - sizeof(unsigned long));
		if(tmpAlrm.structVer != ALARMSTOR_STRUCTVER || tmpAlrm.crc32 != crcCalc)
			break;
		alarms[i] = tmpAlrm.val;
	}
	
	randomSeed(analogRead(2));
}

void loop()
{
	static enum but prevBut = but_NONE;
	static int prev1Val = 0;
	static int prev2Val = 0;
	static char backlight = 0;
	static enum mode mode = mode_NORMAL;
	static char longFlag = 0;
	
	enum but but, realbut;
	
	// Handle interrupt
	if(gotTimer1)
	{
		gotTimer1 = 0;
		tmElements_t tm;
		
		breakTime(now(), tm);
		
		char timestr[] = "now - 15:37   ";
		
		timestr[6] = lookup[tm.Hour/10];
		timestr[7] = lookup[tm.Hour%10];
		timestr[9] = lookup[tm.Minute/10];
		timestr[10] = lookup[tm.Minute%10];
		
		lcd.sendString(0, 0, timestr);
	}
	
	#ifdef DEBUG
	if(Serial.available())
	{
		char chr = Serial.read();
		if(chr == '\x03') // Ctrl-C
		{
			cmdlen = 0;
		}
		else if(chr == '\n')
		{
			sercmd[cmdlen] = 0;
			// handle cmd
			if(!strncmp(sercmd, "SA", 2))
			{ // SA HH:MM FF
				uint8_t hour;
				uint8_t min;
				uint8_t flags;
				sscanf(sercmd, "SA %02hhd:%02hhd %02hhx", &hour, &min, &flags);
				alarms[0].hour = hour;
				alarms[0].min = min;
				alarms[0].enDayEn = flags;
				logobj("Setting Alarm.");
			}
			cmdval = 1;
		}
		else
		{
			if(cmdlen < 19)
			{
				sercmd[cmdlen++] = chr;
			}
		}
	}
	#endif
	
	// Figure out which button is pressed
	int but1Val = analogRead(BUT_PIN1);
	int but2Val = analogRead(BUT_PIN2);
	
	if(histequal(but1Val, prev1Val, 10) && histequal(but2Val, prev2Val, 10))
	{
		if(histequal(but1Val, 1024, 20) && histequal(but2Val, 1024, 20))
			but = but_NONE;
		else if(histequal(but1Val, 0, 70))
			but = but_LEFT;
		else if(histequal(but1Val, 512, 70))
			but = but_RIGHT;
		else if(histequal(but1Val, 682, 70))
			but = but_UP;
		else if(histequal(but2Val, 0, 70))
			but = but_DOWN;
		else if(histequal(but2Val, 512, 70))
			but = but_SELECT;
		else
			but = prevBut;
	}
	else
	{
		but = prevBut;
	}
	
	if(but != but_NONE)
		realButPressed = 1;
	else
		realButPressed = 0;
	
	// Don't send a button event unless a button has been released
	realbut = but_NONE;
	if(but == but_NONE && prevBut != but_NONE) // button released
	{
		if(!longFlag) // Also don't send one if longpress was sent
			realbut = prevBut;
		realButPressed = 0;
		longPressDetected = 0;
		rptPressDetected = 0;
		longFlag = 0;
	}
	
	if(longPressDetected && !longFlag)
	{
		realbut = (enum but)(but + 5);
		longFlag = 1;
	}
	if(rptPressDetected)
	{
		realbut = (enum but)(but + 10); // Unless a repeat should be sent
		rptPressDetected = 0;
	}
	
	// handle buttons globally
	if(realbut == but_SELECT_LONG)
	{
		// logwobj("B5", but2Val);
		realbut = but_NONE;
		backlight = !backlight;
		analogWrite(SCRN_LED_PIN, (backlight) ? 10 : 0);
	}
	
	enum mode modebak = mode;
	
	mode = modeMux(mode, &realbut);
	
	// default actions for buttons
	if(realbut == but_LEFT)
		mode = (enum mode)(mode - 1);
	if(realbut == but_RIGHT)
	{
		mode = (enum mode)(mode + 1);
		// logwobj("default mode++ handler", modebak);
	}
	
	if(mode == mode_NMODES)
	{
		mode = mode_NORMAL;
	}
	if(mode == mode_INVALID)
	{
		// logwobj("default invalid mode handler", modebak);
		mode = mode_DEMO;
	}
	
	if(mode != modebak)
	{
		switch(mode)
		{
			default:
				logwobj("weird mode", mode);
				mode = mode_NORMAL;
				// intentional fallthrough
			case mode_NORMAL:
				lcd.sendString(5, 0, "mode: NORMAL  ");
				break;
			case mode_SETTIME:
				lcd.sendString(5, 0, "mode: SETTIME ");
				break;
			case mode_SETALARM:
				lcd.sendString(5, 0, "mode: SETALARM");
				break;
			case mode_DEMO:
				lcd.sendString(5, 0, "mode: DEMO    ");
				break;
		}
	}
	
	prevBut = but;
	prev1Val = but1Val;
	prev2Val = but2Val;
	#ifdef DEBUG
	if(cmdval)
		cmdlen = 0;
	cmdval = 0;
	#endif
}
