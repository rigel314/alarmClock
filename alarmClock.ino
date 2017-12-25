#include <LCD.h>
#include <TimerOne.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <Wire.h>
#include <avr/pgmspace.h>

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

#define MCP4726_ADDR 0x62

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

#define histequal(a, b, hist) (a > b-hist && a < b+hist)

volatile int gotTimer1 = 0;
LCD lcd(SCRN_SCLK_PIN, SCRN_MOSI_PIN, SCRN_DC_PIN, SCRN_RST_PIN, SCRN_SCE_PIN);

const char sound[] PROGMEM =
#include "bird.h" // This has the data and a semicolon

// Called via timer interrupt
void timer1inter()
{
	gotTimer1 = 1;
}

void print2digits(int number)
{
	if (number >= 0 && number < 10)
		Serial.write('0');
	Serial.print(number);
}

void setup()
{
	Serial.begin(115200); // For debug (owns pins 0 and 1)
	
	Timer1.initialize(1000000);
	Timer1.attachInterrupt(timer1inter);
	
	lcd.initialize();
	lcd.sendCommands("\x21\xB0\x20");
	lcd.sendString(0, 0, "Testing...");
	
	setSyncProvider(RTC.get);
	setTime(RTC.get());
	// setSyncInterval(10);
	
	// TWBR = ((F_CPU / 400000L) - 16) / 2; // Set I2C frequency to 400kHz
	Wire.setClock(400000L);
	
	pinMode(BUT_PIN1, INPUT);
	pinMode(BUT_PIN2, INPUT);
	
	pinMode(RED_PIN, OUTPUT);
	pinMode(GRN_PIN, OUTPUT);
	pinMode(BLU_PIN, OUTPUT);
}

void loop()
{
	static int prevBut = 0;
	static int prev1Val = 0;
	static int prev2Val = 0;
	static int backlight = 0;
	int but;
	
	volatile char blar = sound[0];
	if(blar)
	{
		log("WTF");
	}
	
	if(gotTimer1)
	{
		gotTimer1 = 0;
		tmElements_t tm;
		
		breakTime(now(), tm);
		
		// Serial.print("Ok, Time = ");
		// print2digits(tm.Hour);
		// Serial.write(':');
		// print2digits(tm.Minute);
		// Serial.write(':');
		// print2digits(tm.Second);
		// Serial.print(", Date (D/M/Y) = ");
		// Serial.print(tm.Day);
		// Serial.write('/');
		// Serial.print(tm.Month);
		// Serial.write('/');
		// Serial.print(tmYearToCalendar(tm.Year));
		// Serial.println();
	}
	
	int but1Val = analogRead(BUT_PIN1);
	int but2Val = analogRead(BUT_PIN2);
	
	if(histequal(but1Val, prev1Val, 10) && histequal(but2Val, prev2Val, 10))
	{
		if(histequal(but1Val, 1024, 20) && histequal(but2Val, 1024, 20))
			but = 0;
		else if(histequal(but1Val, 0, 70))
			but = 1;
		else if(histequal(but1Val, 512, 70))
			but = 2;
		else if(histequal(but1Val, 682, 70))
			but = 3;
		else if(histequal(but2Val, 0, 70))
			but = 4;
		else if(histequal(but2Val, 512, 70))
			but = 5;
		else
			but = prevBut;
	}
	else
	{
		but = prevBut;
	}
	
	if(but == 1 && but != prevBut)
	{
		logwobj("B1", but1Val);
		analogWrite(RED_PIN, 1);
		analogWrite(GRN_PIN, 1);
		analogWrite(BLU_PIN, 1);
		RTC.set(SECS_YR_2000);
		setTime(SECS_YR_2000);
	}
	else if(but == 2 && but != prevBut)
	{
		logwobj("B2", but1Val);
		analogWrite(RED_PIN, 32);
		analogWrite(GRN_PIN, 32);
		analogWrite(BLU_PIN, 32);
	}
	else if(but == 3 && but != prevBut)
	{
		logwobj("B3", but1Val);
		analogWrite(RED_PIN, 64);
		analogWrite(GRN_PIN, 64);
		analogWrite(BLU_PIN, 64);
		for(int i = 0; i < 4096; i++)
		{
			Wire.beginTransmission(MCP4726_ADDR);
			// logobj(val);
			Wire.write(i >> 8 & 0xFF);
			Wire.write(i & 0xFF);
			Wire.endTransmission(true);
			delay(1);
		}
	}
	else if(but == 4 && but != prevBut)
	{
		logwobj("B4", but2Val);
		analogWrite(RED_PIN, 128);
		analogWrite(GRN_PIN, 128);
		analogWrite(BLU_PIN, 128);
		
		const char* ptr = sound;
		for(int i = 0; i < 16000; i++)
		{
			Wire.beginTransmission(MCP4726_ADDR);
			int16_t val = pgm_read_byte(ptr++);
			val += 128;
			val <<= 3;
			// logobj(val);
			Wire.write(val >> 8 & 0x07);
			Wire.write(val & 0xFF);
			Wire.endTransmission(true);
			delayMicroseconds(15); // To get almost exactly 8000 samples/sec, which is what the sound was resampled to
		}
		// Wire.endTransmission(true);
	}
	else if(but == 5 && but != prevBut)
	{
		logwobj("B5", but2Val);
		backlight = !backlight;
		analogWrite(SCRN_LED_PIN, (backlight) ? 2 : 0);
	}
	else if(but == 0 && but != prevBut)
	{
		logwobj("no buttons", but1Val);
		logobj(but2Val);
	}
	
	prevBut = but;
	prev1Val = but1Val;
	prev2Val = but2Val;
}
