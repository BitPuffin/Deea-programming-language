#ifndef _EVAL_H
#define _EVAL_H

#include "helper.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>

#include <inttypes.h>
#include <string.h>


static struct
{
	bool running;
} moduleData;

typedef enum
{
	FIXNUM = 0,
	BOOLEAN = 1,
	CHARACTER = 2,
	STRING = 3,
	EMPTY_LIST = 4,
	PAIR = 5,
	SYMBOL = 6

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

bool is_running();
void set_running(bool val);

object* o_is_false;
object* o_is_true;
object* o_empty_list;
object* o_symbol_table;
object* o_quote_symbol;
object* o_ok_symbol;
object* o_set_symbol;
object* o_define_symbol;
object* o_the_empty_environment;
object* o_the_global_environment;

object* allocate_object(void);
object* make_fixnum(long value);

object* cons(object* car, object* cdr);
object* make_symbol(char* value);
object* make_string(char* value);
object* make_character(char value);


object* car(object* pair);
void set_car(object* obj, object* value);
void set_cdr(object* obj, object* value);
object* cdr(object* pair);


bool is_false(object* obj);
bool is_boolean(object* obj);
bool is_true(object* obj);
bool is_assignment(object* exp);
bool is_variable(object* exp);
bool is_definition(object* exp);
bool is_fixnum(object* obj);
bool is_empty_list(object* obj);
bool is_string(object* obj);
bool is_character(object* obj);
bool is_symbol(object* obj);
bool is_delimiter(int c);
bool is_initial(int c);
bool is_pair(object* obj);
bool is_self_evaluating(object *exp);
bool is_tagged_list(object *expression, object *tag);
bool is_quoted(object *expression);
object* text_of_quotation(object *exp);

object* eval_definition(object* exp, object* env);
object* eval_assignment(object* exp, object* env);
object* eval (object* exp, object* env);


object* definition_value(object* exp);
object* definition_variable(object* exp);
object* assignment_value(object* exp);
object* assignment_variable(object* exp);
object* enclosing_environment(object* env);
object* first_frame(object* env);
object* make_frame(object* variables, object* values);
object* frame_variables(object* frame);
object* frame_values(object* frame);
void add_bindings_to_frame(object* var, object* val, object* frame);
object* extend_environment(object* var, object* val, object* base_env);
object* lookup_variable_value(object *var, object *env);
object* setup_environment(void);
void define_variable(object* var, object* val, object* env);
void set_variable_value(object* var, object* val, object* env);



#endif
