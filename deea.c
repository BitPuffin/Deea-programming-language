
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>

#include <inttypes.h>
#include <string.h>

#define DEEA_VER "0.0.1"

#define SAFE_FREE(v) {  if (v) {free(v); v = NULL;}  }
#define car(p) ((p).value.pair->obj[0])
#define cdr(p) ((p).value.pair->obj[1])
/* p stands for predicate */
#define nilp(obj) ((obj).type == ObjType_Nil)

/*
static struct
{
	bool running;
} moduleData;
*/

struct Obj;

typedef struct Obj
{

	enum
	{
		ObjType_Nil,
		ObjType_Pair,
		ObjType_Symbol,
		ObjType_Integer
	} type;

	union
	{
		struct Pair *pair;
		const char* symbol;
		long integer;
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
/************* END OF DECLARATIONS ****************/


int eval_expr(Obj expr, Obj env, Obj* result)
{
	Obj op, args;
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

	op = car(expr);
	args = cdr(expr);

	if (op.type == ObjType_Symbol)
	{
		if (strcmp(op.value.symbol, "QUOTE") == 0)
		{
			if(nilp(args) || !nilp(cdr(args)))
				return Error_Args;

			*result = car(args);
			return Error_OK;
		}
		else if (strcmp(op.value.symbol, "DEFINE") == 0)
		{
			Obj sym,val;

			if (nilp(args) || nilp(cdr(args)) || !nilp(cdr(cdr(args))))
				return Error_Args;

			sym = car(args);
			if (sym.type != ObjType_Symbol)
				return Error_Type;

			err = eval_expr(car(cdr(args)), env, &val);
			if (err)
				return err;

			*result = sym;
			return env_set(env, sym, val);
		}
	}
	return Error_Syntax;
}

int listp(Obj expr)
{
	while (!nilp(expr))
	{
		if (expr.type != ObjType_Pair)
		{
			return 0;
		}
		expr = cdr(expr);
	}
	return 1;
}

Obj env_create(Obj parent)
{
	return cons(parent, nil);
}

int env_get(Obj env, Obj symbol, Obj* result)
{
	Obj parent = car(env);
	Obj bs = cdr(env);

	while(!nilp(bs))
	{
		Obj b = car(bs);
		if (car(b).value.symbol == symbol.value.symbol)
		{
			*result = cdr(b);
			return Error_OK;
		}
		bs = cdr(bs);
	}
	if (nilp(parent))
		return Error_Unbound;

	return env_get(parent, symbol, result);
}

int env_set(Obj env, Obj symbol, Obj value)
{
	Obj bs = cdr(env);
	Obj b = nil;

	while (!nilp(bs))
	{
		b = car(bs);
		if (car(b).value.symbol == symbol.value.symbol)
		{
			cdr(b) = value;
			return Error_OK;
		}
		bs = cdr(bs);
	}

	b = cons(symbol, value);
	cdr(env) = cons(b, cdr(env));

	return Error_OK;
}

int lexer(const char* str, const char** start, const char** end)
{

	const char* ws = " \t\n";
	const char* delim = "() \t\n";
	const char* prefix = "()";

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

			cdr(p) = item;

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
			cdr(p) = cons(item, nil);
			p = cdr(p);
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

	car(o) = car_val;
	cdr(o) = cdr_val;

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
		a = car(p);
		if (strcmp(a.value.symbol, s) == 0)
			return a;

		p = cdr(p);
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
			print_expr(car(o));
			o = cdr(o);
			while (!nilp(o))
			{
				if (o.type == ObjType_Pair)
				{
					putchar(' ');
					print_expr(car(o));
					o = cdr(o);
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























