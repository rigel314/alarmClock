#include <Arduino.h>
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

#include "modes.h"

static const char* fields[] = {"year", "month", "day", "hour", "minute", "second"};
static const uint8_t min[] = {0, 1, 1, 0, 0, 0};
static uint8_t max[] = {255, 12, 31, 23, 59, 59};

#define LEAP_YEAR(Y) ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )

static const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0

enum mode normmode(enum mode mode, enum but* butp)
{
	(void) butp;
	return mode;
}

enum mode stimemode(enum mode mode, enum but* butp)
{
	static char entry = 0;
	static tmElements_t tm;
	int len;
	
	enum but but = *butp;
	
	if(!entry && but == but_SELECT)
	{
		*butp = but_NONE;
		entry = 1;
		breakTime(now(), tm);
		char str[15];
		len = snprintf(str, 15, "%s: %04d", fields[0], (int)tm.Year + 1970);
		for(int i = len; i < 14; i++)
			str[i] = ' ';
		str[14] = '\0';
		lcd.sendString(4, 0, str);
		
		return mode;
	}
	
	if(entry)
	{
		char str[15];
		
		uint8_t* fld = &((uint8_t*)&tm)[7-entry-(entry > 3)];  // index into the correct field in the tmElements_t struct
		// if(but != but_NONE)
		// 	logwobj("offset", (int)(fld - (uint8_t*)&tm));
		int val = (int)*fld;
		int off = 0;
		const char* fmt = "%s: %02d";
		if(entry == 1)
		{
			off = 1970; // Adjust year
			fmt = "%s: %04d";
		}
		
		max[2] = monthDays[tm.Month - 1];
		if(tm.Month == 2 && LEAP_YEAR(tm.Year))
		{
			max[2] = 29;
		}
		if(tm.Day > max[2])
		{
			tm.Day = max[2];
		}
		
		switch(but)
		{
			case but_LEFT:
				entry--;
				break;
				
			case but_SELECT:
				entry = 6;
				// Intentional fallthrough
			case but_RIGHT:
				entry++;
				if(entry == 7)
				{
					time_t tme = makeTime(tm);
					RTC.set(tme);
					setTime(tme);
					entry = 0;
				}
				break;
			
			case but_UP:
			case but_UP_RPT:
				if(val < max[entry - 1])
					(*fld)++;
				val = *fld;
				val += off; // Adjust year
				len = snprintf(str, 15, fmt, fields[entry - 1], val);
				for(int i = len; i < 14; i++)
					str[i] = ' ';
				str[14] = '\0';
				lcd.sendString(4, 0, str);
				break;
			
			case but_DOWN:
			case but_DOWN_RPT:
				if(val > min[entry - 1])
					(*fld)--;
				val = *fld;
				val += off; // Adjust year
				len = snprintf(str, 15, fmt, fields[entry - 1], val);
				for(int i = len; i < 14; i++)
					str[i] = ' ';
				str[14] = '\0';
				lcd.sendString(4, 0, str);
				break;
				
			default:
				break;
		}
		if((but == but_LEFT || but == but_RIGHT) && entry)
		{
			off = 0;
			fmt = "%s: %02d";
			if(entry == 1)
			{
				off = 1970; // Adjust year
				fmt = "%s: %04d";
			}
			
			fld = &((uint8_t*)&tm)[7-entry-(entry > 3)];
			val = *fld;
			val += off;
			len = snprintf(str, 15, fmt, fields[entry - 1], val);
			for(int i = len; i < 14; i++)
				str[i] = ' ';
			str[14] = '\0';
			lcd.sendString(4, 0, str);
		}
		if(entry == 0)
		{
			lcd.sendString(4, 0, (char*)emptyLine);
		}

		
		*butp = but_NONE;
	}
	
	return mode;
}

enum mode salrmmode(enum mode mode, enum but* butp)
{
	(void) butp;
	return mode;
}

enum mode demomode(enum mode mode, enum but* butp)
{
	enum but but = *butp;
	
	if(but == but_UP)
	{
		*butp = but_NONE;
		
		unsigned char r = 0;
		unsigned char g = 0;
		unsigned char b = 0;
		
		lcd.sendString(4, 0, "running demo1 ");
		
		for(int i = 0; i < 128; i++)
		{
			analogWrite(RED_PIN, r);
			analogWrite(GRN_PIN, g);
			analogWrite(BLU_PIN, b);
			delay(100);
			r++;
		}
		for(int i = 128; i < 228; i++)
		{
			analogWrite(RED_PIN, r);
			analogWrite(GRN_PIN, g);
			analogWrite(BLU_PIN, b);
			delay(100);
			r++;
			g++;
		}
		for(int i = 228; i < 256; i++)
		{
			analogWrite(RED_PIN, r);
			analogWrite(GRN_PIN, g);
			analogWrite(BLU_PIN, b);
			delay(100);
			r++;
			g++;
			b++;
		}
		
		delay(2000);
		
		analogWrite(RED_PIN, 0);
		analogWrite(GRN_PIN, 0);
		analogWrite(BLU_PIN, 0);
		
		lcd.sendString(4, 0, (char*)emptyLine);
	}
	if(but == but_DOWN)
	{
		*butp = but_NONE;
		
		lcd.sendString(4, 0, "running demo2 ");
		
		const char* ptr = sound;
		for(int i = 0; i < 16000; i++)
		{
			Wire.beginTransmission(MCP4726_ADDR);
			int16_t val = (int8_t)pgm_read_byte(ptr++);
			val += 128;
			val <<= 3;
			// logobj(val);
			Wire.write(val >> 8);// & 0x07);
			Wire.write(val & 0xFF);
			Wire.endTransmission(true);
			delayMicroseconds(15);
		}
		
		lcd.sendString(4, 0, (char*)emptyLine);
	}
	
	return mode;
}
