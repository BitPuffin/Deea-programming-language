#ifndef _READER_H
#define _READER_H

#include "eval.h"
#include "helper.h"

object* read(FILE* in);
object* read_pair(FILE* in);
object* read_character(FILE* in);

#endif
