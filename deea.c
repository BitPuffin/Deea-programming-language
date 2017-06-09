
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#include <inttypes.h>
#include <string.h>

#define DEEA_VER "0.0.1"

#define SAFE_FREE(v) {  if (v) {free(v); v = NULL;}  }

static struct
{
	bool running;
}moduleData;

struct Obj;

/*
 * Holds data
 */
typedef struct
{
	int type;
	int value;
} Obj;

enum
{
	E_NIL = 1,
	E_OPEN_PAREN = 2,
	E_CLOSED_PAREN = 3,
	E_TRUE = 4,
	E_FALSE = 5,
	E_INT = 6,
	E_FUNCTION = 7,
	E_SUM = 8,
};

static void read(Obj* o);

/*
 * Allocated a new object of a certain type
 * and sets the size for it
 */
static Obj* alloc(int type)
{
	Obj* o = malloc(sizeof(Obj));
	o->value = 0;
	o->type = type;

	return o;
}

static void read_line(Obj* o)
{
	while (moduleData.running)
	{
		o->type = E_OPEN_PAREN;
		read(o);
	}
}

static void end_line(Obj* o)
{
	while (moduleData.running)
	{
		if (o->type == E_SUM)
		{
			printf("%d \n", o->value);
			o->value = 0;
		}
		o->type = E_CLOSED_PAREN;
		read(o);
	}
}

static void parse_integer(Obj* o, int c)
{
	while (moduleData.running)
	{
		if (o->type == E_SUM)
		{
			o->value += (c - '0');
			//printf("%s %d \n", "We parsed integer: ", o->value);
			read(o);
		}
	}
}

static void parse_symbol(Obj* o, int c)
{
	while (moduleData.running)
	{
		if (c == '+')
		{
			o->type = E_SUM;
			read(o);
		}
	}
}

/*
 * Read the input from console and interpretate it
 */
static void read(Obj* o)
{

	while(moduleData.running)
	{
		int c = getchar();

		if (c == EOF)
			return;

		if (c == '(')
			read_line(o);

		if (c == ')')
			end_line(o);

		if (c == '\n' || c == '\r' || c == '\t' || c == ' ')
			continue;

		if (isdigit(c))
			parse_integer(o, c);

		if (isalpha(c) || strchr("+", c))
			parse_symbol(o, c);

		printf("%s \n", "Can't process the last command given!");
		moduleData.running = false;
	}

}

int main(int argc, char* argv[])
{

	moduleData.running = true;

	printf("%s %s %s \n", "Deea version:", DEEA_VER, "has started. ");

	for (int i = 1; i < argc; i++)
	{
		if ( (strcmp(argv[i], "--version") == 0 ) || strcmp(argv[i], "-version"))
			printf("%s \n", DEEA_VER);

	}

	char buffer[256];

	Obj* o = alloc(E_NIL);

	/* Main loop */
	while (moduleData.running)
	{

		read(o);

		while (fgets(buffer, 256, stdin))
		{
			if (strcmp(buffer, "quit"))
			{
				moduleData.running = false;
				break;
			}
		}
	}

	SAFE_FREE(o);

	return 0;
}
