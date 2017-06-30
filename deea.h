#ifndef _DEEA_H
#define _DEEA_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>

#include <inttypes.h>
#include <string.h>

#define SAFE_FREE(v) {  if (v) {free(v); v = NULL;}  }

#define DEEA_VER "0.0.1"

struct
{
	bool running;
} moduleData;

typedef enum
{
	FIXNUM,
	BOOLEAN,
	CHARACTER,
	STRING,
	EMPTY_LIST,
	PAIR,
	SYMBOL

} object_type;

typedef struct object {

	object_type type;

	union
	{

		struct
		{
			bool value;
		} boolean;

		struct
		{
			char* value;
		} string;

		struct
		{
			char* value;
		} symbol;

		struct
		{
			char value;
		} character;

		struct
		{
			long value;
		} fixnum;

		struct
		{
			struct object *car;
			struct object *cdr;
		} pair;

	} data;

} object;


object* o_is_false;
object* o_is_true;
object* o_empty_list;
object* o_symbol_table;


object* allocate_object(void);
object* make_fixnum(long value);


bool is_false(object* obj);
bool is_boolean(object* obj);
bool is_true(object* obj);


bool is_fixnum(object* obj);
bool is_empty_list(object* obj);
bool is_string(object* obj);
bool is_symbol(object* obj);
bool is_delimiter(int c);
bool is_initial(int c);

object* cons(object* car, object* cdr);
object* make_symbol(char* value);
object* make_string(char* value);
object* make_character(char value);


object* car(object* pair);
void set_car(object* obj, object* value);
void set_cdr(object* obj, object* value);
object* cdr(object* pair);


void write_pair(object* obj);
void write(object* obj);


object* eval (object* exp);


object* read(FILE* in);
object* read_pair(FILE* in);
object* read_character(FILE* in);


void peek_expected_delimiter(FILE* in);
int peek(FILE* in);


void eat_whitespace(FILE* in);
void eat_string(FILE* in, char* str);

void init(void);

#endif




