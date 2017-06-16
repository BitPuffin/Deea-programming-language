/*
#   deea programming language
#
#   Copyright (C) 2017 Muresan Vlad
#
#   This project is free software; you can redistribute it and/or modify it
#   under the terms of the MIT license. See LICENSE.md for details.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>

#include <inttypes.h>
#include <string.h>

#define DEEA_VER "0.0.1"

#define SAFE_FREE(v) {  if (v) {free(v); v = NULL;}  }
#define p1(p) ((p).value.pair->obj[0])
#define p2(p) ((p).value.pair->obj[1])
/* p stands for predicate */
#define nilp(obj) ((obj).type == ObjType_Nil)

struct Obj;

typedef int (*Builtin) (struct Obj, struct Obj* result);

typedef struct Obj
{

	enum
	{
		ObjType_Nil,
		ObjType_Pair,
		ObjType_Symbol,
		ObjType_Integer,
		ObjType_Builtin,
		ObjType_Closure
	} type;

	union
	{
		struct Pair *pair;
		const char* symbol;
		long integer;
		Builtin builtin;
	} value;

} Obj;

struct Pair
{
	struct Obj obj[2];
};

static const Obj nil = { ObjType_Nil };
static Obj sym_table = {ObjType_Nil };

typedef enum
{
	Error_OK = 0,
	Error_Syntax = 1,
	/*
	 * Attempted to evaluate a symbol
	 * for which no bindings exists
	 */
	Error_Unbound = 2,
	/*
	 * A list expression was shorter or longer
	 * than expected
	 */
	Error_Args = 3,
	/*
	 * An object in a expression was of
	 * a different type than expected
	 */
	Error_Type = 4,
} Error;


/************* START OF DECLARATIONS ****************/
int read_expr(const char* input, const char** end, Obj* result);
int read_list(const char* start, const char** end, Obj* result);
int parse_simple(const char* start, const char* end, Obj* result);
int lexer(const char* str, const char** start, const char** end);
int listp(Obj expr);

Obj env_create(Obj parent);
int env_set(Obj env, Obj symbol, Obj value);
int env_get(Obj env, Obj symbol, Obj* result);

int eval_expr(Obj expr, Obj env, Obj* result);

char* _strdup(const char* s);
Obj make_int(long x);
Obj make_sym(const char* s);
Obj cons(Obj car_val, Obj cdr_val);

Obj make_builtin(Builtin fn);
Obj copy_list(Obj list);
int apply(Obj fn, Obj args, Obj* result);

int make_closure(Obj env, Obj args, Obj body, Obj* result);

int builtin_numless(Obj args, Obj* result);
int builtin_numgr(Obj args, Obj* result);
int builtin_numlessoreq(Obj args, Obj* result);
int builtin_numgroreq(Obj args, Obj* result);
int builtin_numeq(Obj args, Obj* result);
int builtin_p1(Obj args, Obj* result);
int builtin_p2(Obj args, Obj* result);
int builtin_cons(Obj args, Obj* result);
int builtin_add(Obj args, Obj* result);
int builtin_sub(Obj args, Obj* result);
int builtin_div(Obj args, Obj* result);
int builtin_mul(Obj args, Obj* result);
/************* END OF DECLARATIONS ****************/

/************* BUILT IN FUNCTIONS WHICH DO STUFF ************/
int builtin_p1(Obj args, Obj* result)
{

	if(nilp(args) || !nilp(p2(args)))
		return Error_Args;

	if (nilp(p1(args)))
		*result = nil;
	else if (p1(args).type != ObjType_Pair)
		return Error_Type;
	else
		*result = p1(p1(args));

	return Error_OK;
}


int builtin_p2(Obj args, Obj* result)
{

	if (nilp(args) || !nilp(p2(args)))
		return Error_Args;

	if (nilp(p1(args)))
		*result = nil;
	else if (p1(args).type != ObjType_Pair)
		return Error_Type;
	else
		*result = p2(p1(args));

	return Error_OK;
}


int builtin_numless(Obj args, Obj* result)
{
	Obj a, b;

	if (nilp(args) || nilp(p2(args)) || !nilp(p2(p2(args))))
		return Error_Args;

	a = p1(args);
	b = p1(p2(args));

	if (a.type != ObjType_Integer || b.type != ObjType_Integer)
		return Error_Type;

	* result = (a.value.integer < b.value.integer) ? make_sym("T") : nil;

	return Error_OK;
}


int builtin_numgr(Obj args, Obj* result)
{
	Obj a, b;

	if (nilp(args) || nilp(p2(args)) || !nilp(p2(p2(args))))
		return Error_Args;

	a = p1(args);
	b = p1(p2(args));

	if (a.type != ObjType_Integer || b.type != ObjType_Integer)
		return Error_Type;

	* result = (a.value.integer > b.value.integer) ? make_sym("T") : nil;

	return Error_OK;
}


int builtin_numlessoreq(Obj args, Obj* result)
{
	Obj a, b;

	if (nilp(args) || nilp(p2(args)) || !nilp(p2(p2(args))))
		return Error_Args;

	a = p1(args);
	b = p1(p2(args));

	if (a.type != ObjType_Integer || b.type != ObjType_Integer)
		return Error_Type;

	* result = (a.value.integer <= b.value.integer) ? make_sym("T") : nil;

	return Error_OK;
}


int builtin_numgroreq(Obj args, Obj* result)
{
	Obj a, b;

	if (nilp(args) || nilp(p2(args)) || !nilp(p2(p2(args))))
		return Error_Args;

	a = p1(args);
	b = p1(p2(args));

	if (a.type != ObjType_Integer || b.type != ObjType_Integer)
		return Error_Type;

	* result = (a.value.integer >= b.value.integer) ? make_sym("T") : nil;

	return Error_OK;
}


int builtin_numeq(Obj args, Obj* result)
{
	Obj a, b;

	if (nilp(args) || nilp(p2(args)) || !nilp(p2(p2(args))))
		return Error_Args;

	a = p1(args);
	b = p1(p2(args));

	if (a.type != ObjType_Integer || b.type != ObjType_Integer)
		return Error_Type;

	* result = (a.value.integer == b.value.integer) ? make_sym("T") : nil;

	return Error_OK;

}

int builtin_add(Obj args, Obj* result)
{
	Obj a, b;

	if (nilp(args) || nilp(p2(args)) || !nilp(p2(p2(args))))
		return Error_Args;

	a = p1(args);
	b = p1(p2(args));

	if (a.type != ObjType_Integer || b.type != ObjType_Integer)
		return Error_Type;

	*result = make_int (a.value.integer + b.value.integer);

	return Error_OK;
}

int builtin_sub(Obj args, Obj* result)
{
	Obj a, b;

	if (nilp(args) || nilp(p2(args)) || !nilp(p2(p2(args))))
		return Error_Args;

	a = p1(args);
	b = p1(p2(args));

	if (a.type != ObjType_Integer || b.type != ObjType_Integer)
		return Error_Type;

	*result = make_int (a.value.integer - b.value.integer);

	return Error_OK;
}

int builtin_mul(Obj args, Obj* result)
{
	Obj a, b;

	if (nilp(args) || nilp(p2(args)) || !nilp(p2(p2(args))))
		return Error_Args;

	a = p1(args);
	b = p1(p2(args));

	if (a.type != ObjType_Integer || b.type != ObjType_Integer)
		return Error_Type;

	*result = make_int (a.value.integer * b.value.integer);

	return Error_OK;
}

int builtin_div(Obj args, Obj* result)
{
	Obj a, b;

	if (nilp(args) || nilp(p2(args)) || !nilp(p2(p2(args))))
		return Error_Args;

	a = p1(args);
	b = p1(p2(args));

	if (a.type != ObjType_Integer || b.type != ObjType_Integer)
		return Error_Type;

	*result = make_int (a.value.integer / b.value.integer);

	return Error_OK;
}


int builtin_cons(Obj args, Obj* result)
{

	if (nilp(args) || nilp(p2(args)) || !nilp(p2(p2(args))))
		return Error_Args;

	*result = cons(p1(args), p1(p2(args)));

	return Error_OK;
}
/***************** END OF BUILT IN FUNCTION WHICH DO STUFF ****************/

int make_closure(Obj env, Obj args, Obj body, Obj* result)
{
	Obj p;

	if (!listp(body))
		return Error_Syntax;

	/* Check argument names are all symbols */
	p = args;
	while(!nilp(p))
	{

		if (p.type == ObjType_Symbol)
			break;
		else if (p.type != ObjType_Pair || p1(p).type != ObjType_Symbol)
			return Error_Type;

		p = p2(p);
	}

	*result = cons(env, cons(args, body));
	/* Override default 'Pair' type */
	result->type = ObjType_Closure;

	return Error_OK;

}

Obj make_builtin(Builtin fn)
{
	Obj a;
	a.type = ObjType_Builtin;
	a.value.builtin = fn;
	return a;
}

Obj copy_list(Obj list)
{
	Obj a, p;
	if (nilp(list))
		return nil;

	a = cons(p1(list), nil);
	p = a;
	list = p2(list);

	while(!nilp(list))
	{
		p2(p) = cons(p1(list), nil);
		p = p2(p);
		list = p2(list);
	}

	return a;
}

int apply(Obj fn, Obj args, Obj* result)
{

	Obj env, args_names, body;

	if (fn.type == ObjType_Builtin)
		return (*fn.value.builtin)(args, result);
	else if (fn.type != ObjType_Closure)
		return Error_Type;

	env = env_create(p1(fn));
	args_names = p1(p2(fn));
	body = p2(p2(fn));

	/* Bind arguments */
	while (!nilp(args_names))
	{

		if (args_names.type == ObjType_Symbol)
		{
			env_set(env, args_names, args);
			args = nil;
			break;
		}

		if (nilp(args))
			return Error_Args;

		env_set(env, p1(args_names), p1(args));
		args_names = p2(args_names);

		args = p2(args);
	}

	if (!nilp(args))
		return Error_Args;

	/* Evaluate the body */
	while(!nilp(body))
	{
		Error err = eval_expr(p1(body), env, result);
		if(err)
			return err;

		body = p2(body);
	}

	return Error_OK;

}

int eval_expr(Obj expr, Obj env, Obj* result)
{
	Obj op, args, p;
	Error err;

	if (expr.type == ObjType_Symbol)
	{
		return env_get(env, expr, result);
	}
	else if (expr.type != ObjType_Pair)
	{
		*result = expr;
		return Error_OK;
	}

	if (!listp(expr))
		return Error_Syntax;

	op = p1(expr);
	args = p2(expr);

	if (op.type == ObjType_Symbol)
	{
		if (strcmp(op.value.symbol, "QUOTE") == 0)
		{
			if(nilp(args) || !nilp(p2(args)))
				return Error_Args;

			*result = p1(args);
			return Error_OK;
		}
		else if (strcmp(op.value.symbol, "LAMBDA") == 0)
		{
			if (nilp(args) || nilp(p2(args)))
				return Error_Args;

			return make_closure(env, p1(args), p2(args), result);
		}
		else if (strcmp(op.value.symbol, "IF") == 0)
		{
			Obj cond, val;

			if (nilp(args) || nilp(p2(args)) || nilp(p2(p2(args))) || !nilp(p2(p2(p2(args)))))
				return Error_Args;

			err = eval_expr(p1(args), env, &cond);
			if (err)
				return err;

			val = nilp(cond) ? p1(p2(p2(args))) : p1(p2(args));
			return eval_expr(val, env, result);

		}
		else if (strcmp(op.value.symbol, "DEFINE") == 0)
		{
			Obj sym, val;

			if (nilp(args) || nilp(p2(args)) || !nilp(p2(p2(args))))
				return Error_Args;

			sym = p1(args);

			if (sym.type == ObjType_Pair)
			{
				err = make_closure(env, p2(sym), p2(args), &val);
				sym = p1(sym);
				if (sym.type != ObjType_Symbol)
					return Error_Type;
			} else if (sym.type == ObjType_Symbol)
			{
				if (!nilp(p2(args)))
					return Error_Args;
				err = eval_expr(p1(p2(args)), env, &val);
			} else
				return Error_Type;

			if (err)
				return err;

			*result = sym;
			return env_set(env, sym, val);
		}
	}

	/* Evaluate operators */
	err = eval_expr(op, env, &op);
	if (err)
		return err;

	/* Evaluate arguments */
	args = copy_list(args);
	p = args;
	while(!nilp(p))
	{
		err = eval_expr(p1(p), env, &p1(p));
		if (err)
			return err;

		p = p2(p);
	}

	return apply(op, args, result);
}

int listp(Obj expr)
{
	while (!nilp(expr))
	{
		if (expr.type != ObjType_Pair)
		{
			return 0;
		}
		expr = p2(expr);
	}
	return 1;
}

Obj env_create(Obj parent)
{
	return cons(parent, nil);
}

int env_get(Obj env, Obj symbol, Obj* result)
{
	Obj parent = p1(env);
	Obj bs = p2(env);

	while(!nilp(bs))
	{
		Obj b = p1(bs);
		if (p1(b).value.symbol == symbol.value.symbol)
		{
			*result = p2(b);
			return Error_OK;
		}
		bs = p2(bs);
	}
	if (nilp(parent))
		return Error_Unbound;

	return env_get(parent, symbol, result);
}

int env_set(Obj env, Obj symbol, Obj value)
{
	Obj bs = p2(env);
	Obj b = nil;

	while (!nilp(bs))
	{
		b = p1(bs);
		if (p1(b).value.symbol == symbol.value.symbol)
		{
			p2(b) = value;
			return Error_OK;
		}
		bs = p2(bs);
	}

	b = cons(symbol, value);
	p2(env) = cons(b, p2(env));

	return Error_OK;
}

int lexer(const char* str, const char** start, const char** end)
{

	const char* ws = " \t\n";
	const char* delim = "() \t\n";
	const char* prefix = "()\'";

	str += strspn(str, ws);

	if (str[0] == '\0')
	{
		*start = *end = NULL;
		return Error_Syntax;
	}

	*start = str;

	if (strchr(prefix, str[0] ) != NULL)
	{

		*end = str+1;
	}
	else
	{
		*end = str + strcspn(str, delim);
	}

	return Error_OK;
}


int read_expr(const char* input, const char** end, Obj* result)
{

	const char* token;
	Error err;

	err = lexer(input, &token, end);
	if (err)
		return err;

	if (token[0] == '(')
	{
		return read_list(*end, end, result);
	}
	else if (token[0] == ')')
	{
		return Error_Syntax;
	}
	else if (token[0] == '\'')
	{
		*result = cons(make_sym("QUOTE"), cons (nil,nil));
		return read_expr(*end, end, &p1(p2(*result)));
	}
	else
	{
		return parse_simple(token, *end, result);
	}
}

/*
 * Used for:
 * integers, symbols , nil
 */
int parse_simple(const char* start, const char* end, Obj* result)
{
	char* buf, *p;

	/* Integer? */
	long val = strtol(start, &p, 10);
	if (p == end)
	{
		result->type = ObjType_Integer;
		result->value.integer = val;
		return Error_OK;
	}

	/* nil or symbol */
	buf = malloc(end - start+ 1);
	p = buf;

	while (start != end)
	{
		*p++ = toupper(*start);
		++start;
	}
	*p = '\0';

	if (strcmp(buf, "NIL") == 0)
		*result = nil;
	else
		*result = make_sym(buf);

	free(buf);

	return Error_OK;
}

int read_list(const char* start, const char** end, Obj* result)
{

	Obj p;

	*end = start;
	p = *result = nil;

	for (;;)
	{
		const char* token;
		Obj item;
		Error err;

		err = lexer(*end, &token, end);

		if (err)
			return err;

		if (token [0] == ')')
			return Error_OK;

		if (token [0] == '.' && *end - token == 1)
		{
			/* Not a good list */
			if (nilp(p))
				return Error_Syntax;

			err = read_expr(*end, end, &item);

			if (err)
				return err;

			p2(p) = item;

			/* Read the closing ')' */
			err = lexer(*end, &token, end);
			if (!err && token[0] != ')')
				err = Error_Syntax;

			return err;
		}

		err = read_expr(token, end, &item);
		if (err)
			return err;

		if (nilp(p))
		{
			/* First item */
			*result = cons(item, nil);
			p = *result;
		}
		else
		{
			p2(p) = cons(item, nil);
			p = p2(p);
		}

	}
}

/*
 * Allocate a pair on heap and assign its two elements
 */
Obj cons(Obj car_val, Obj cdr_val)
{
	Obj o;

	o.type = ObjType_Pair;
	o.value.pair = malloc(sizeof(struct Pair));

	p1(o) = car_val;
	p2(o) = cdr_val;

	return o;
}

Obj make_int(long x)
{
	Obj a;
	a.type = ObjType_Integer;
	a.value.integer = x;
	return a;
}

char* _strdup(const char* s)
{
	char* d = malloc(strlen(s) + 1);
	if (d == NULL) return NULL;
	strcpy(d, s);

	return d;
}

Obj make_sym(const char* s)
{
	Obj a, p;

	p = sym_table;
	/*
	 * Here we check for another symbol
	 * with the same name as 's' and
	 * if it exists then we return it
	 * and not allocate new memory to
	 * the heap
	 */
	while (!nilp(p))
	{
		a = p1(p);
		if (strcmp(a.value.symbol, s) == 0)
			return a;

		p = p2(p);
	}

	a.type = ObjType_Symbol;
	a.value.symbol = _strdup(s);
	sym_table = cons(a, sym_table);

	return a;
}

/*
 * Example of pair:
 * ( a . b)
 * a = car
 * b = cdr
 */

void print_expr(Obj o)
{
	switch(o.type)
	{
		case ObjType_Nil:
			printf("NIL");
			break;
		case ObjType_Pair:
			putchar('(');
			print_expr(p1(o));
			o = p2(o);
			while (!nilp(o))
			{
				if (o.type == ObjType_Pair)
				{
					putchar(' ');
					print_expr(p1(o));
					o = p2(o);
				}
				else
				{
					printf(" . ");
					print_expr(o);
					break;
				}
			}
			putchar(')');
			break;
		case ObjType_Symbol:
			printf("%s \n", o.value.symbol);
			break;
		case ObjType_Integer:
			printf("%ld \n", o.value.integer);
			break;
		case ObjType_Builtin:
			printf("#<BUILTIN:%p>", o.value.builtin);
			break;
	}
}

int main(int argc, char* argv[])
{

	/*
	Obj o;

	o = make_int(42);
	print_expr(o);

	o = make_sym("FOO");
	print_expr(o);

	o = cons(make_int(1),  cons(make_int(2), cons(make_int(3), nil)));
	print_expr(o);
	*/

	//char* input = "(42 foo)";
	char input[256];

	Obj env;
	env = env_create(nil);

	/* Set initial env */
	env_set(env, make_sym("CAR"), make_builtin(builtin_p1));
	env_set(env, make_sym("CDR"), make_builtin(builtin_p2));
	env_set(env, make_sym("CONS"), make_builtin(builtin_cons));
	env_set(env, make_sym("+"), make_builtin(builtin_add));
	env_set(env, make_sym("-"), make_builtin(builtin_sub));
	env_set(env, make_sym("*"), make_builtin(builtin_mul));
	env_set(env, make_sym("/"), make_builtin(builtin_div));
	env_set(env, make_sym("T"), make_sym("T"));
	env_set(env, make_sym("="), make_builtin(builtin_numeq));
	env_set(env, make_sym("<"), make_builtin(builtin_numless));
	env_set(env, make_sym(">"), make_builtin(builtin_numgr));
	env_set(env, make_sym(">="), make_builtin(builtin_numgroreq));
	env_set(env, make_sym("<="), make_builtin(builtin_numlessoreq));

	while (fgets(input, 256, stdin) != NULL)
	{
		const char* p = input;
		Error err;
		Obj expr, result;

		err = read_expr(p, &p, &expr);

		if (!err)
			err = eval_expr(expr, env, &result);

		switch (err)
		{
			case Error_OK:
				print_expr(result);
				putchar('\n');
				break;
			case Error_Syntax:
				puts("Syntax error");
				break;
			case Error_Unbound:
				puts("Symbol not bound");
				break;
			case Error_Args:
				puts("Wrong number of arguments");
				break;
			case Error_Type:
				puts("Wrong type");
				break;
		}
	}

	return 0;
}






















