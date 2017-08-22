
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
		AtomType_Symbol
	} type;

	const char* symbol;
	long integer;
	struct Pair* pair;

};

struct Pair
{
	struct Atom atom[2];
};

typedef struct Atom Atom;

static const Atom nil = { AtomType_Nil };

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
	Atom a;
	a.type = AtomType_Symbol;
	a.symbol = _strdup(value);

	return a;
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
			printf("\n");
			print_expr(car(atom));
			atom = cdr(atom);

			while(!nilc(atom))
			{
				if (atom.type == AtomType_Pair)
					{
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
	printf("\n");
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

	return EXIT_SUCCESS;
}









