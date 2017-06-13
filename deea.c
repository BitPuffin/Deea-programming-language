
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

static struct
{
	bool running;
}moduleData;

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

Obj make_sym(const char* s)
{
	Obj a, p;

	p = sym_table;
	while (!nilp(p))
	{
		a = car(p);
		if (strcmp(a.value.symbol, s) == 0)
			return a;

		p = cdr(p);
	}

	a.type = ObjType_Symbol;
	a.value.symbol = s;
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

	Obj o;

	o = make_int(42);
	print_expr(o);

	o = make_sym("FOO");
	print_expr(o);

	o = cons(make_int(1),  cons(make_int(2), cons(make_int(3), nil)));
	print_expr(o);

	return 0;
}























