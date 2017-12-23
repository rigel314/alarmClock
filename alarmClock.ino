#include <LCD.h>
#include <TimerOne.h>

#define B1_PIN A0
#define B2_PIN A1
#define B3_PIN A2
#define B4_PIN A3
#define B5_PIN 2

#define RED_PIN 11
#define GRN_PIN 5
#define BLU_PIN 3

#define SCRN_SCLK_PIN 7
#define SCRN_MOSI_PIN 8
#define SCRN_DC_PIN 9
#define SCRN_RST_PIN 10
#define SCRN_SCE_PIN 12
#define SCRN_LED_PIN 6 // PWM

#define _FILENAME_ "alarmClock.ino"
#define _XSTRINGIFY(x) #x
#define STRINGIFY(x) _XSTRINGIFY(x)

#define log(msg, ...) \
					do \
					{ \
						Serial.print(_FILENAME_ ":" STRINGIFY(__LINE__) " in `"); \
						Serial.print(__PRETTY_FUNCTION__); \
						Serial.println("`, " msg ##__VA_ARGS__); \
					} \
					while(0)

volatile int gotTimer1 = 0;
LCD lcd(SCRN_SCLK_PIN, SCRN_MOSI_PIN, SCRN_DC_PIN, SCRN_RST_PIN, SCRN_SCE_PIN);

// Called via timer interrupt
void timer1inter()
{
	gotTimer1 = 1;
}

void setup()
{
	Serial.begin(115200); // For debug (owns pins 0 and 1)
	
	Timer1.initialize(100000);
	Timer1.attachInterrupt(timer1inter);
	
	lcd.initialize();
	lcd.sendCommands("\x21\xB0\x20");
	lcd.sendString(0, 0, "Testing...");
	
	pinMode(B1_PIN, INPUT);
	pinMode(B2_PIN, INPUT);
	pinMode(B3_PIN, INPUT);
	pinMode(B4_PIN, INPUT);
	pinMode(B5_PIN, INPUT);
	
	pinMode(RED_PIN, OUTPUT);
	pinMode(GRN_PIN, OUTPUT);
	pinMode(BLU_PIN, OUTPUT);
}

void loop()
{
	static int prevBut = 0;
	int but;
	
	if(gotTimer1)
	{
		gotTimer1 = 0;
	}
	
	but = (analogRead(B1_PIN) > 512) ? 1 : 0;
	but = (analogRead(B2_PIN) > 512) ? 2 : but;
	but = (analogRead(B3_PIN) > 512) ? 3 : but;
	but = (analogRead(B4_PIN) > 512) ? 4 : but;
	but = (digitalRead(B5_PIN)) ? 5 : but;
	
	if(but == 1 && but != prevBut)
	{
		log("B1");
		analogWrite(RED_PIN, 1);
		analogWrite(GRN_PIN, 1);
		analogWrite(BLU_PIN, 1);
	}
	else if(but == 2 && but != prevBut)
	{
		log("B2");
		analogWrite(RED_PIN, 32);
		analogWrite(GRN_PIN, 32);
		analogWrite(BLU_PIN, 32);
	}
	else if(but == 3 && but != prevBut)
	{
		log("B3");
		analogWrite(RED_PIN, 64);
		analogWrite(GRN_PIN, 64);
		analogWrite(BLU_PIN, 64);
	}
	else if(but == 4 && but != prevBut)
	{
		log("B4");
		analogWrite(RED_PIN, 128);
		analogWrite(GRN_PIN, 128);
		analogWrite(BLU_PIN, 128);
	}
	else if(but == 5 && but != prevBut)
	{
		log("B5");
		analogWrite(RED_PIN, 255);
		analogWrite(GRN_PIN, 255);
		analogWrite(BLU_PIN, 255);
	}
	
	prevBut = but;
}
