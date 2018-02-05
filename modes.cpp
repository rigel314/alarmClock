#include <Arduino.h>
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

#include "modes.h"

static const char* fields[] = {"year", "month", "day", "hour", "minute", "second"};
static const uint8_t min[] = {0, 1, 1, 0, 0, 0};
static uint8_t max[] = {255, 12, 31, 23, 59, 59};
const char* days = "SMTWHFS";

#define LEAP_YEAR(Y) ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )

static const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0

time_t alarm2time(char hour, char min)
{
	tmElements_t tm1, tm2;
	time_t nowtime;
	
	nowtime = now();
	breakTime(nowtime, tm1);
	
	tm1.Hour = hour;
	tm1.Minute = min;
	tm1.Second = 0;
	breakTime(nowtime + SECS_PER_DAY, tm2);
	
	if(makeTime(tm1) > nowtime - ALARM_DURATION)
		return makeTime(tm1);
	else
		return makeTime(tm2);
}

inline time_t secsUntilAlarm(time_t alarmTime)
{
	return alarmTime - now();
}

enum mode normmode(enum mode mode, enum but* butp)
{
	static char submode = 0;
	static char prevsubmode = -1;
	static char alarmWaiting = 0;
	static uint8_t prevr = 0, prevg = 0, prevb = 0;
	
	char update = 0;
	enum but but = *butp;
	time_t until = 86400;
	
	// look for next alarm
	for(int i = 0; i < nALARMS; i++)
	{
		if(alarms[i].enDayEn & 0x80)
		{
			time_t altm = alarm2time(alarms[i].hour, alarms[i].min);
			tmElements_t tm;
			
			breakTime(altm, tm);
			if(alarms[i].enDayEn & 1 << (7-tm.Wday))
				until = min(until, secsUntilAlarm(altm));
		}
	}
	
	// cycle through modes
	if(but == but_SELECT && submode >= 1 && submode <= 3)
	{
		submode++;
		*butp = but_NONE;
	}
	
	uint8_t r, g, b;
	r = g = b = 0;
	
	if(until < ALARM_DURATION && !alarmWaiting)
	{
		if(submode == 0)
			submode = 1;
		
		if(until > ALARM_DURATION - ALARM_LED_DURATION)
		{
			r = 255L*ALARM_DURATION/(ALARM_LED_DURATION) - 255L*until/(ALARM_LED_DURATION);
			if(r > 127)
			{
				g = r - 127;
			}
			if(g > 63)
			{
				b = g - 63;
			}
		}
		else
		{
			r = 255;
			g = 127;
			b = 63;
		}
		
		// log("rgb");
		// logobj(r);
		// logobj(g);
		// logobj(b);
	}
	else
	{
		r = 255;
		g = 127;
		b = 63;
		if(submode == 4)
		{
			logobj("here");
			submode = 0;
			alarmWaiting = 0;
		}
		if(submode > 0)
		{
			alarmWaiting = 1;
		}
	}
	
	if(r != prevr || g != prevg || b != prevb || submode != prevsubmode)
		update = 1;
	
	if(update)
	{
		logobj((int) submode);
		switch(submode)
		{
			case 0:
				lcd.sendString(4, 0, emptyLine);
				analogWrite(RED_PIN, 0);
				analogWrite(GRN_PIN, 0);
				analogWrite(BLU_PIN, 0);
				break;
			case 1:
				lcd.sendString(4, 0, "normal alarm  ");
				analogWrite(RED_PIN, r);
				analogWrite(GRN_PIN, g);
				analogWrite(BLU_PIN, b);
				break;
			case 2:
				lcd.sendString(4, 0, "no ambiance   ");
				analogWrite(RED_PIN, r);
				analogWrite(GRN_PIN, g);
				analogWrite(BLU_PIN, b);
				break;
			case 3:
				lcd.sendString(4, 0, "no noise      ");
				analogWrite(RED_PIN, r);
				analogWrite(GRN_PIN, g);
				analogWrite(BLU_PIN, b);
				break;
			case 4:
				lcd.sendString(4, 0, "no light&noise");
				analogWrite(RED_PIN, 0);
				analogWrite(GRN_PIN, 0);
				analogWrite(BLU_PIN, 0);
				break;
			default:
				break;
		}
	}
	
	prevsubmode = submode;
	
	return mode;
}

enum mode stimemode(enum mode mode, enum but* butp)
{
	static char entry = 0;
	static tmElements_t tm;
	char len;
	
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
		
		uint8_t* fld = &((uint8_t*)&tm)[7-entry-(entry > 3)]; // index into the correct field in the tmElements_t struct
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
	static char entry = 0;
	static char newentry = 0;
	static int alarmChoice;
	static char val = 0;
	// static char newval = 0;
	static struct alarm alrm = {0,0,0};
	
	char len;
	enum but but = *butp;
	
	// begin entry
	if(!entry && but == but_SELECT)
	{
		char str[19];
		*butp = but_NONE;
		entry = 1;
		alarmChoice = 0;
		alrm = alarms[alarmChoice];
		if(nALARMS == 1) // Maybe make this an #if
		{
			entry++;
			val = alrm.hour;
			len = snprintf(str, 19, "set: %s%02d%s:%02d", LCD::reverseVideoOn, (int)alrm.hour, LCD::reverseVideoOff, (int)alrm.min);
		}
		else
		{
			val = alarmChoice;
			len = snprintf(str, 19, "alarm: %d", (int)alarmChoice+1);
		}
		for(int i = len; i < 18; i++)
			str[i] = ' ';
		str[18] = '\0';
		lcd.sendString(4, 0, str);
		
		return mode;
	}
	
	// actually do entry
	if(entry)
	{
		// logwobj("but IN:", *butp);
		char str[19];
		char str3[15];
		newentry = entry;
		// newval = val;
		
		// handle buttons
		switch(but)
		{
			case but_LEFT:
				newentry--;
				if(nALARMS == 1 && newentry == 1)
					newentry--;
				break;
				
			case but_SELECT:
				newentry = 11;
				// Intentional fallthrough
			case but_RIGHT:
				newentry++;
				if(newentry == 12)
				{
					newentry = 0;
				}
				break;
			
			case but_UP:
			case but_UP_RPT:
				val++;
				break;
			
			case but_DOWN:
			case but_DOWN_RPT:
				val--;
				break;
				
			default:
				break;
		}
		
		char entrychanged = (entry != newentry);
		char shft = (6-(entry-5)); // bit shift for weekday selection
		
		// update storage variable on entry change for the entry you just left
		if(entrychanged)
		{
			switch(entry)
			{
				case 1:
					alarmChoice = val;
					break;
				case 2:
					alrm.hour = val;
					break;
				case 3:
					alrm.min = val;
					break;
				case 4:
					alrm.enDayEn = (alrm.enDayEn & 0x7F) | val<<7;
					break;
				case 5:
				case 6:
				case 7:
				case 8:
				case 9:
				case 10:
				case 11:
					alrm.enDayEn = (alrm.enDayEn & ~(1 << shft)) | (val << shft);
					// logwobj("enDayEn", (int)alrm.enDayEn, HEX);
					break;
				default:
					break;
			}
		}
		
		entry = newentry;
		// val = newval;
		
		strcpy(str3, emptyLine);
		
		char len;
		shft = (6-(entry-5)); // calculate this again on the new entry
		
		// create correct line 4 for newentry and newval
		// update temp var from storage var on entry change for the entry you just changed to
		switch(entry)
		{
			case 1:
				if(entrychanged)
					val = alarmChoice;
				if(val < 1)
					val = nALARMS;
				if(val > nALARMS)
					val = 1;
				len = snprintf(str, 19, "alarm: %s%d%s", LCD::reverseVideoOn, val, LCD::reverseVideoOff);
				break;
			case 2:
				if(entrychanged)
					val = alrm.hour;
				if(val < 0)
					val = 23;
				if(val > 23)
					val = 0;
				len = snprintf(str, 19, "set: %s%02d%s:%02d", LCD::reverseVideoOn, (int)val, LCD::reverseVideoOff, (int)alrm.min);
				break;
			case 3:
				if(entrychanged)
					val = alrm.min;
				if(val < 0)
					val = 59;
				if(val > 59)
					val = 0;
				len = snprintf(str, 19, "set: %02d:%s%02d%s", (int)alrm.hour, LCD::reverseVideoOn, (int)val, LCD::reverseVideoOff);
				break;
			case 4:
				if(entrychanged)
					val = !!(alrm.enDayEn & 0x80);
				if(val < 0)
					val = 1;
				if(val > 1)
					val = 0;
				len = snprintf(str, 19, "enable: %s%s%s", LCD::reverseVideoOn, (val) ? "yes" : "no", LCD::reverseVideoOff);
				break;
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
				if(entrychanged)
					val = !!(alrm.enDayEn & (1 << shft));
				if(val < 0)
					val = 1;
				if(val > 1)
					val = 0;
				len = 16;
				strcpy(str, "on: ");
				strcpy(str3, "off:");
				for(int i = 0; i < 7; i++)
				{
					uint8_t curval = (alrm.enDayEn & ~(1 << shft)) | (val << shft);
					if(i == entry-5)
						memcpy(&str[4+i], LCD::reverseVideoOn, 2);
					str[4+i+2*(i >= entry-5)+2*(i > entry-5)] = (curval & (1 << (6-i))) ? days[i] : ' ';
					str3[4+i] = (curval & (1 << (6-i))) ? ' ' : days[i]; // create line 3 for disabled weekdays
					if(i == entry-5)
						memcpy(&str[4+i+3], LCD::reverseVideoOff, 2);
				}
				for(int i = 12; i < 14; i++)
					str3[i] = ' ';
				str3[14] = '\0';
				break;
			default:
				len = 0;
				entry = 0;
				break;
		}
		
		// Pad string with spaces until end of line and display
		if(but != but_NONE && entry)
		{
			for(int i = len; i < 18; i++)
				str[i] = ' ';
			str[18] = '\0';
			lcd.sendString(4, 0, str);
			lcd.sendString(3, 0, str3);
		}
		
		// Erase lines 3 and 4
		if(entry == 0)
		{
			if(but == but_SELECT || but == but_RIGHT)
			{
				// update real variable
				alarms[alarmChoice] = alrm;
				log("new alarm");
				logobj((int)alarms[alarmChoice].hour);
				logobj((int)alarms[alarmChoice].min);
				logobj((int)alarms[alarmChoice].enDayEn, HEX);
				// TODO: write alarms to EEPROM/RTCram
			}
			lcd.sendString(4, 0, (char*)emptyLine);
			lcd.sendString(3, 0, (char*)emptyLine);
		}
		
		// inform generic handler that this mode handled the button
		*butp = but_NONE;
		// logwobj("but OUT:", *butp);
	}
	
	return mode;
}

enum mode demomode(enum mode mode, enum but* butp)
{
	#ifndef DEBUG
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
			Wire.write(val >> 8); // & 0x07);
			Wire.write(val & 0xFF);
			Wire.endTransmission(true);
			delayMicroseconds(15);
		}
		
		lcd.sendString(4, 0, (char*)emptyLine);
	}
	
	#endif

	return mode;
}
