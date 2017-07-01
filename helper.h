#ifndef _HELPER_H
#define _HELPER_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "eval.h"

void peek_expected_delimiter(FILE* in);
int peek(FILE* in);

void eat_whitespace(FILE* in);
void eat_string(FILE* in, char* str);

#endif
