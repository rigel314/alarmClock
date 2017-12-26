#include <Arduino.h>
#include <Wire.h>

#include "modes.h"

enum mode modeMux(enum mode mode, enum but but)
{
	// if(but != but_NONE)
	// 	logwobj("button", but);
	
	switch(mode)
	{
		case mode_NORMAL:
			mode = normmode(mode, but);
			break;
		case mode_SETTIME:
			mode = normmode(mode, but);
			break;
		case mode_SETALARM:
			mode = normmode(mode, but);
			break;
		case mode_DEMO:
			mode = demomode(mode, but);
			break;
		default:
			break;
	}
	
	if(mode == mode_NMODES)
	{
		mode = mode_NORMAL;
	}
	if(mode == mode_INVALID)
	{
		mode = mode_DEMO;
	}
	
	return mode;
}

enum mode normmode(enum mode mode, enum but but)
{
	if(but == but_LEFT)
		mode = (enum mode)(mode - 1);
	if(but == but_RIGHT)
		mode = (enum mode)(mode + 1);
	
	return mode;
}

enum mode demomode(enum mode mode, enum but but)
{
	if(but == but_LEFT)
		mode = (enum mode)(mode - 1);
	if(but == but_RIGHT)
		mode = (enum mode)(mode + 1);
	if(but == but_UP)
	{
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
		
		lcd.sendString(4, 0, "              ");
	}
	if(but == but_DOWN)
	{
		lcd.sendString(4, 0, "running demo2 ");
		
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
			delayMicroseconds(15);
		}
		
		lcd.sendString(4, 0, "              ");
	}
	
	return mode;
}
