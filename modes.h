#ifndef _MODES_H
#define _MODES_H

#include "common.h"

enum mode normmode(enum mode mode, enum but* butp);
enum mode demomode(enum mode mode, enum but* butp);
enum mode stimemode(enum mode mode, enum but* butp);
enum mode salrmmode(enum mode mode, enum but* butp);

inline enum mode modeMux(enum mode mode, enum but* butp)
{
	// if(*butp != but_NONE)
	// 	logwobj("button", *butp);
	
	switch(mode)
	{
		case mode_NORMAL:
			mode = normmode(mode, butp);
			break;
		case mode_SETTIME:
			mode = stimemode(mode, butp);
			break;
		case mode_SETALARM:
			mode = salrmmode(mode, butp);
			break;
		case mode_DEMO:
			mode = demomode(mode, butp);
			break;
		default:
			break;
	}
	
	return mode;
}

#endif // _MODES_H
