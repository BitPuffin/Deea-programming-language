
/*
 * Big thanks for helping me out to: BitPuffin http://bitpuffin.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define car(p) ((p).pair->atom[0])
#define cdr(p) ((p).pair->atom[1])
#define nilc(p) ((p).type == AtomType_Nil)

static char* _strdup(const char* value)
{
	size_t len = strlen(value);
	char* buffer = malloc(len + 1);
	strcpy(buffer, value);
	buffer[len] = '\0';

	return buffer;
}

struct Atom
{
	enum
	{
		AtomType_Nil,
		AtomType_Integer,
		AtomType_Pair,
		AtomType_Symbol,
	} type;

	const char* symbol;
	long integer;
	struct Pair* pair;

};

typedef enum
{
	Error_OK = 0,
	Error_Syntax = 1,
	Error_Unbound = 3,
	Error_Args = 4,/* A list exp was shorter or longer than expected */
	Error_Type = 5 /* An object in an exp was of different type than expected */

} Error;

struct Pair
{
	struct Atom atom[2];
};

typedef struct Atom Atom;

static const Atom nil = { AtomType_Nil };
static Atom sym_table = { AtomType_Nil };

/* Allocate a pair on the heap and assign its two elements. */
static Atom cons(Atom car_value, Atom cdr_value)
{
	Atom p;

	p.type = AtomType_Pair;
	p.pair = malloc(sizeof(struct Pair));

	car(p) = car_value;
	cdr(p) = cdr_value;

	return p;
}

static Atom make_int(long value)
{
	Atom a;
	a.type = AtomType_Integer;
	a.integer = value;

	return a;
}

static Atom make_symbol(const char* value)
{
	Atom a, p;

	p = sym_table;

	/*
	 * Keep track of all symbols created
	 * and return the same created atom
	 * if found.
	 */
	while (!nilc(p))
	{
		a = car(p);
		if (strcmp(a.symbol, value) == 0)
			return a;

		p = cdr(p);
	}

	/*
	 * If we have not found the symbol
	 * then it means we have a new one
	 * so we  must create it and store it.
	 */
	a.type = AtomType_Symbol;
	a.symbol = _strdup(value);

	sym_table = cons(a, sym_table);


	return a;
}

/* Read a single object and return error status and a pointer to the reminder of the input. */
static int read_expr(const char* input, const char** end, Atom* result);

/*
 * Returns the start and the end of the next token in a string
 * eg: (foo bar) -> |(| |foo| |bar| |)| , 4 different tokens
 */
static int lexer(const char* str, const char** start, const char** end)
{
	const char* ws = " \t\n";
	const char* delim = " ()\t\n";
	const char* prefix = "()";

	str += strspn(str, ws);
	/*
	 * In the example from above str[0] will be:
	 * (
	 * f
	 * f
	 * b
	 * b
	 * )
	 */


	/*
	 * If it hits the end of a string
	 * without finding a token return a syntax error.
	 */
	if (str[0] == '\0')
	{
		*start = *end = NULL;
		return Error_Syntax;
	}

	*start = str;

	/*
	 * Look for () and return what comes after ( or )
	 */
	if (strchr(prefix, str[0]) != NULL)
	{
		*end = str + 1;
	}
	else
	{
		*end = str + strcspn(str, delim);
	}

	return Error_OK;
}

static int parse_simple(const char* start, const char* end, Atom* result)
{

	char *buf, *p;

	/* Integer? */
	long val = strtol(start, &p, 10);

	if (p == end)
	{
		result->type = AtomType_Integer;
		result->integer = val;

		return Error_OK;
	}

	/* NIL or symbol */
	buf = malloc(sizeof(end - start + 1));

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
		*result = make_symbol(buf);

	free(buf);

	return Error_OK;
}

static int read_list(const char* start, const char** end, Atom* result)
{
	Atom p;

	*end = start;
	p = *result = nil;

	for (;;)
	{
		const char* token;
		Atom item;
		Error err;

		err = lexer(*end, &token, end);
		if (err)
			return err;

		if (token[0] == ')')
			return Error_OK;

		if (token[0] == '.' && *end - token == 1)
		{
			/* Improper list */
			if (nilc(p))
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

		if (nilc(p))
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

	return Error_OK;
}

static int read_expr(const char* input, const char** end, Atom* result)
{
	const char* token;
	Error err;

	err = lexer(input, &token, end);
	if (err)
		return err;

	if (token[0] == '(')
		return read_list(*end, end, result);
	else if (token[0] == ')')
		return Error_Syntax;
	else
		return parse_simple(token, *end, result);
}

/*
 * HOW AN ENVIRONMENT WORKS ?
 *
 * To associate identifiers with objects we
 * need an environment. This binds each
 * identifier with its corresponding value.
 * eg:
 * Identifier | Value
 * FOO        |  97
 * BAR        |  NIL
 * BOR        | (X Y Z)
 *
 * (parent ( identifier . value ) ... )
 */

/*
 * Create an empty environment with a specific parent (can be NIL)
 */
static Atom env_create(Atom parent)
{
	return cons(parent, nil);
}

static int env_get(Atom env, Atom symbol, Atom* result)
{

	Atom parent =  car(env);
	Atom next = cdr(env);

	while (!nilc(next))
	{
		Atom b = car(next);
		if (car(b).symbol == symbol.symbol)
		{
			*result = cdr(b);
			return Error_OK;
		}
		next = cdr(next);
	}

	/*
	 * eg: foo , here the indentifier,foo, needs a value!
	 */
	if (nilc(parent))
		return Error_Unbound;

	return env_get(parent, symbol, result);
}

static int env_set(Atom env, Atom symbol, Atom value)
{

	Atom next = cdr(env);
	Atom b = nil;

	/*
	 * First look if we already have
	 * this symbol stored so we can
	 * change it's value
	 */
	while (!nilc(next))
	{

		b = car(next);

		if (b.symbol == symbol.symbol)
		{
			car(b) = value;
			return Error_OK;
		}

		next = cdr(next);
	}

	b = cons(symbol, value);
	cdr(env) = cons(b, cdr(env));

	return Error_OK;
}

static int listc(Atom expr)
{
	while(nilc(expr))
	{

		if (expr.type != AtomType_Pair)
			return 0;

		expr = cdr(expr);
	}

	return 1;
}

static int eval_expr(Atom expr, Atom env, Atom* result)
{

	Atom op, args;
	Error err;

	if (expr.type == AtomType_Symbol)
	{
		return env_get(env, expr, result);
	}
	else if (expr.type != AtomType_Pair)
	{
		*result = expr;
		return Error_OK;
	}

	if (!listc(expr))
		return Error_Syntax;

	op = car(expr);
	args = cdr(expr);

	if (op.type == AtomType_Symbol)
	{
		if ( strcmp(op.symbol, "QUOTE") == 0 )
		{
			/*
			 * eg:
			 * 1) quote - no arguments so fail
			 * 2) quote foo (bar pep)) - too many arguments
			 */
			if (nilc(args) || !nilc(cdr(args)))
				return Error_Args;

			*result = car(args);
			return Error_OK;

		}
		else if ( strcmp(op.symbol, "DEFINE") == 0 )
		{
			//if (nilc(args) || nilc) TODO
		}
	}

	return Error_Syntax;
}

static void print_expr(Atom atom)
{
	switch (atom.type)
	{
		case AtomType_Nil:
			printf("NIL");
			break;

		case AtomType_Integer:
			printf("%ld", atom.integer);
			break;

		case AtomType_Symbol:
			printf("%s", atom.symbol);
			break;

		case AtomType_Pair:
			putchar('(');
			print_expr(car(atom));
			atom = cdr(atom);

			while(!nilc(atom))
			{
				if (atom.type == AtomType_Pair)
				{
					putchar(' ');
					print_expr(car(atom));
					atom = cdr(atom);
				}
				else
				{
					printf(" . ");
					print_expr(atom);
					break;
				}
			}

			putchar(')');
			break;

		default:
			break;
	}
}

int main(int argc, char* argv[])
{

	/*
	 * Test unit

	Atom a;
	a = make_int(1997);
	print_expr(a);

	a = make_symbol("Hello");
	print_expr(a);

	a = cons(
			make_int(1),
			cons(
				make_int(2),
				cons(make_int(3),
			nil
			)));
	print_expr(a);
	*/

	Atom env;

	env = env_create(nil);

	char buffer[1024];
	while (fgets(buffer, 1024, stdin) != NULL)
	{
		Error err;
		Atom expr, result;

		const char* p = buffer;

		err = read_expr(p, &p, &expr);

		if (!err)
			err = eval_expr(expr, env, &result);

		switch(err)
		{
			case Error_OK:
				print_expr(result);
				putchar('\n');
				break;
			case Error_Syntax:
				puts("Syntax error!");
				break;
			case Error_Unbound:
				puts("Symbol not bound!");
				break;
			case Error_Args:
				puts("Wrong number of arguments!");
				break;
			case Error_Type:
				puts("Wrong type!");
				break;
		}

	}

	return EXIT_SUCCESS;
}









