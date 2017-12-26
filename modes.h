#ifndef _MODES_H
#define _MODES_H

#include "common.h"

enum mode modeMux(enum mode mode, enum but* butp);

enum mode normmode(enum mode mode, enum but* butp);
enum mode demomode(enum mode mode, enum but* butp);
enum mode stimemode(enum mode mode, enum but* butp);
enum mode salrmmode(enum mode mode, enum but* butp);

#endif // _MODES_H
