#include "eval.h"
#include "helper.h"

bool is_running()
{
	return moduleData.running;
}

void set_running(bool val)
{
	moduleData.running = val;
}


object *allocate_object(void)
{
	object* obj;

	obj = malloc(sizeof(object));
	if (obj == NULL)
	{
		fprintf(stderr, "out of memory!");
	}
	return obj;
}

object* make_fixnum(long value)
{
	object* o;

	o = allocate_object();
	o->type = FIXNUM;
	o->data.fixnum.value = value;

	return o;
}

object* make_character(char value)
{
	object* obj;

	obj = allocate_object();
	obj->type = CHARACTER;
	obj->data.character.value = value;

	return obj;
}

object* make_string(char* value)
{
	object* obj;

	obj = allocate_object();
	obj->type = STRING;
	obj->data.string.value = malloc(strlen(value) + 1);
	if (obj->data.string.value == NULL)
	{
		fprintf(stderr, "out of memory for allocatin string \n");
		moduleData.running = false;
	}
	strcpy(obj->data.string.value, value);
	return obj;
}

object* make_symbol(char* value)
{
	object* obj;
	object* element;

	element = o_symbol_table;
	while (!is_empty_list(element))
	{
		if (strcmp(car(element)->data.symbol.value, value) == 0)
			return car(element);

		element = cdr(element);
	}

	obj = allocate_object();
	obj->type = SYMBOL;
	obj->data.symbol.value = malloc(strlen(value) + 1);
	if (obj->data.symbol.value == NULL)
	{
		fprintf(stderr, "out of memory for allocating symbol\n");
		moduleData.running = false;
	}

	strcpy(obj->data.symbol.value, value);
	o_symbol_table = cons(obj, o_symbol_table);

	return obj;
}


object* cons(object* car, object* cdr)
{
	object* obj;

	obj = allocate_object();
	obj->type = PAIR;
	obj->data.pair.car = car;
	obj->data.pair.cdr = cdr;

	return obj;
}

object* car(object* pair)
{
	return pair->data.pair.car;
}

void set_car(object* obj, object* value)
{
	obj->data.pair.car = value;
}

object* cdr(object* pair)
{
	return pair->data.pair.cdr;
}

void set_cdr(object* obj, object* value)
{
	obj->data.pair.cdr = value;
}

bool is_initial(int c)
{
	return (
			isalpha(c) || c == '*' || c == '/' || c == '>'
			|| c == '<' || c == '?' || c == '!');
}

bool is_delimiter(int c)
{
	return isspace(c) || c == EOF || c == '(' || c == ')' ||
		c == ';' || c == '"';
}

bool is_string(object* obj)
{
	return obj->type == STRING;
}

bool is_boolean(object* obj)
{
	return (obj->type == BOOLEAN);
}

bool is_false(object* obj)
{
	return obj == o_is_false;
}

bool is_true(object* obj)
{
	return obj == o_is_true;
}

bool is_symbol(object* obj)
{
	return obj->type == SYMBOL;
}

bool is_empty_list(object* obj)
{
	return obj == o_empty_list;
}

bool is_fixnum(object* obj)
{
	return (obj->type == FIXNUM);
}


bool is_character(object* obj)
{
	return obj->type == CHARACTER;
}

bool is_pair(object* obj)
{
	return obj->type == PAIR;
}

bool is_self_evaluating(object* exp)
{
	return (
			is_boolean(exp) ||
			is_fixnum(exp) ||
			is_string(exp) ||
			is_character(exp)
			);
}

bool is_tagged_list(object* expression, object* tag)
{
	object* the_car;

	if (is_pair(expression))
	{
		the_car = car(expression);
		return is_symbol(the_car) && (the_car == tag);
	}

	return false;
}

bool is_quoted(object *expression)
{
	return is_tagged_list(expression, o_quote_symbol);
}


bool is_definition(object* exp)
{
	return is_tagged_list(exp, o_define_symbol);
}

bool is_assignment(object* exp)
{
	return is_tagged_list(exp, o_set_symbol);
}

bool is_variable(object* exp)
{
	return is_symbol(exp);
}

object *assignment_variable(object *exp) {
    return car(cdr(exp));
}

object *assignment_value(object *exp) {
    return car(cdr(cdr(exp)));
}

object* definition_variable(object* exp) {
    return car(cdr(exp));
}

object* definition_value(object* exp) {
    return car(cdr(cdr(exp)));
}

object* enclosing_environment(object* env)
{
	return cdr(env);
}


object* first_frame(object* env)
{
	return car(env);
}


object* make_frame(object* variables, object* values)
{
	return cons(variables, values);
}


object* frame_variables(object* frame)
{
	return car(frame);
}


object* frame_values(object* frame)
{
	return cdr(frame);
}


void add_bindings_to_frame(object* var, object* val, object* frame)
{
	set_car(frame, cons(var, car(frame)));
	set_cdr(frame, cons(val, cdr(frame)));
}


object* extend_environment(object* var, object* val, object* base_env)
{
	return cons(make_frame(var, val), base_env);
}


object* lookup_variable_value(object *var, object *env)
{

	object* frame;
	object* vars;
	object* val;

	while (!is_empty_list(env))
	{
		frame = first_frame(env);
		vars = frame_variables(env);
		val = frame_values(env);

		while (!is_empty_list(vars))
		{
			if (var == car(vars))
				return car(val);

			vars = cdr(vars);
			val = cdr(val);
		}

		env = enclosing_environment(env);
	}

	fprintf(stderr, "unbound variable \n");
	moduleData.running = false;
}


object* setup_environment(void)
{
	object* initial_env;

	initial_env = extend_environment(
			o_empty_list,
			o_empty_list,
			o_the_empty_environment);
	return initial_env;
}


void define_variable(object* var, object* val, object* env)
{
	object* frame;
    object* vars;
    object* vals;

    frame = first_frame(env);
    vars = frame_variables(frame);
    vals = frame_values(frame);

    while (!is_empty_list(vars)) {
        if (var == car(vars)) {
            set_car(vals, val);
            return;
        }
        vars = cdr(vars);
        vals = cdr(vals);
    }

    add_bindings_to_frame(var, val, frame);
}


void set_variable_value(object* var, object* val, object* env)
{
	object* frame;
	object* vars;
	object* vals;

	while (!is_empty_list(env))
	{
		frame = first_frame(env);
		vars = frame_variables(env);
		vals = frame_values(env);

		while(!is_empty_list(vars))
		{
			if (var == car(vars))
			{
				set_car(vals, val);
				return;
			}
			vars = cdr(vars);
			vals = cdr(vals);
		}
		env = enclosing_environment(env);
	}
	fprintf(stderr, "unbound variable\n");
	moduleData.running = false;
}



object* eval_assignment(object* exp, object* env) {
    set_variable_value(assignment_variable(exp),
                       eval(assignment_value(exp), env),
                       env);
    return o_ok_symbol;
}

object* eval_definition(object* exp, object* env) {
	define_variable(definition_variable(exp),
                    eval(definition_value(exp), env),
                    env);
    return o_ok_symbol;
}

object* text_of_quotation(object *exp)
{
	//printf("%d \n", car(exp)->type);
	//printf("%d \n", car(cdr(exp))->type);
	/*
	 * Scenario:
	 * (quote (deea))
	 * where:
	 * quote -> symbol, predefined
	 * deea -> pair, interpreted as a symbol in read()
	 */
	return (car(cdr(exp)));
}

object* eval (object* exp, object* env)
{

	if (is_self_evaluating(exp))
		return exp;
	/*
	 * We basically check for a specific symbol, in this case 'quote',
	 * and if we find it then we get the value of the 'quote' which
	 * is going to be another symbol
	 */
	else if (is_quoted(exp))
		return text_of_quotation(exp);

	else if (is_definition(exp))
		return eval_definition(exp, env);

	else if (is_assignment(exp))
		return eval_assignment(exp, env);

	else if (is_variable(exp))
		return lookup_variable_value(exp, env);

	else
	{
		fprintf(stderr, "cannot eval unknown expression \n");
		moduleData.running = false;
	}


	return exp;
}
