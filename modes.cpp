#include "modes.h"

enum mode modeMux(enum mode mode, enum but but)
{
	if(but != but_NONE)
		logwobj("button", but);
	return mode;
}
