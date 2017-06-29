#include "deea.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>

#include <inttypes.h>
#include <string.h>

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
	PAIR

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

static object *allocate_object(void)
{
	object* obj;

	obj = malloc(sizeof(object));
	if (obj == NULL)
	{
		fprintf(stderr, "out of memory!");
	}
	return obj;
}

static object* make_fixnum(long value)
{
	object* o;

	o = allocate_object();
	o->type = FIXNUM;
	o->data.fixnum.value = value;

	return o;
}

static bool is_boolean(object* obj)
{
	return (obj->type == BOOLEAN);
}

static bool is_false(object* obj)
{
	return obj == o_is_false;
}

static bool is_true(object* obj)
{
	return obj == o_is_true;
}

static bool is_empty_list(object* obj)
{
	return obj == o_empty_list;
}

static bool is_fixnum(object* obj)
{
	return (obj->type == FIXNUM);
}

static object* make_character(char value)
{
	object* obj;

	obj = allocate_object();
	obj->type = CHARACTER;
	obj->data.character.value = value;

	return obj;
}

static object* make_string(char* value)
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

static object* cons(object* car, object* cdr)
{
	object* obj;

	obj = allocate_object();
	obj->type = PAIR;
	obj->data.pair.car = car;
	obj->data.pair.cdr = cdr;

	return obj;
}

static object* car(object* pair)
{
	return pair->data.pair.car;
}

void set_car(object* obj, object* value)
{
	obj->data.pair.car = value;
}

static object* cdr(object* pair)
{
	return pair->data.pair.cdr;
}

void set_cdr(object* obj, object* value)
{
	obj->data.pair.cdr = value;
}

static bool is_string(object* obj)
{
	return obj->type == STRING;
}

static void init(void)
{
	o_is_false = allocate_object();
	o_is_false->type = BOOLEAN;
	o_is_false->data.boolean.value = false;

	o_is_true = allocate_object();
	o_is_true->type = BOOLEAN;
	o_is_true->data.boolean.value = true;

	o_empty_list = allocate_object();
	o_empty_list->type = EMPTY_LIST;

}

static bool is_delimiter(int c)
{
	return isspace(c) || c == EOF || c == '(' || c == ')' ||
		c == ';' || c == '"';
}

static int peek(FILE* in)
{
	int c;

	c = getc(in);
	ungetc(c, in);

	return c;
}

static void eat_string(FILE* in, char* str)
{
	int c;

	while(*str != '\0')
	{
		c = getc(in);
		if (c != *str)
		{
			fprintf(stderr, "unexpected character '%c'\n", c);
			moduleData.running = false;
		}
		str++;
	}
}

static void eat_whitespace(FILE* in)
{
	int c;

	while((c = getc(in)) != EOF)
	{
		if (isspace(c))
			continue;
		else if (c == ';') {
			/* treat it like a whitespace */
			while (((c = getc(in)) != EOF) && (c != '\n'));
			continue;
		}

		ungetc(c, in);
		break;
	}
}

static void peek_expected_delimiter(FILE* in)
{
	if (!is_delimiter(peek(in)))
	{
		fprintf(stderr, "character not followed by delimiter \n");
		moduleData.running = false;
	}
}

static object* read_character(FILE* in)
{
	int c;

	c = getc(in);

	switch(c)
	{
		case EOF:
			fprintf(stderr, "incomplete character literal \n");
			moduleData.running = false;
			break;
		case 's':
			if (peek(in) == 'p')
			{
				eat_string(in, "pace");
				peek_expected_delimiter(in);
				return make_character(' ');
			}
			break;

		case 'n':
			if (peek(in) == 'e')
			{
				eat_string(in, "ewline");
				peek_expected_delimiter(in);
				return make_character('\n');
			}
			break;
	}

	peek_expected_delimiter(in);
	return make_character(c);
}

static object* read(FILE* in);

object* read_pair(FILE* in)
{
	int c;

	object* car_obj;
	object* cdr_obj;

	eat_whitespace(in);

	c = getc(in);

	if (c == ')') /* empty list */
		return o_empty_list;

	ungetc(c, in);

	car_obj = read(in);

	eat_whitespace(in);

	c = getc(in);

	if (c == '.') /* read improper list */
	{
		c = peek(in);
		if (!is_delimiter(c))
		{
			fprintf(stderr, "dot not followed by delimitator\n");
			moduleData.running = false;
		}

		cdr_obj = read(in);
		eat_whitespace(in);
		c = getc(in);

		if (c != ')')
		{
			fprintf(stderr, "missing right paren\n");
			moduleData.running = false;
		}
		return cons(car_obj, cdr_obj);
	}
	else
	{
		/* read list */
		ungetc(c, in);
		cdr_obj = read_pair(in);
		return cons(car_obj, cdr_obj);
	}

}

object* read(FILE* in)
{

	int c;
	int8_t sign = 1;
	long num = 0;
	int i;

	#define BUFFER_MAX 1000
	char buffer[BUFFER_MAX];

	eat_whitespace(in);

	c = getc(in);


	if (c == '#')
	{
		/* read boolean or character */
		c = getc(in);
		switch(c)
		{
			case 't':
				return o_is_true;
				break;

			case 'f':
				return o_is_false;
				break;

			case '\\':
				return read_character(in);

			default:
				fprintf(stderr, "unknown boolean or character literal \n");

				moduleData.running = false;
				break;
		}
	}
	else if (c == '"')
	{
		/* read string */
		i = 0;
		while((c = getc(in)) != '"')
		{
			if (c == '\\')
			{
				c = getc(in);
				if (c == 'n')
					c = '\n';
			}

			if (c == EOF)
			{
				fprintf(stderr, "non-terminated string literal \n");
				moduleData.running = false;
			}
			if (i < BUFFER_MAX - 1)
				buffer[i++] = c; // we want to keep BUFFER_MAX to \0
			else
			{
				fprintf(stderr, "string too long. Maximum length is %d\n", BUFFER_MAX);
				moduleData.running = false;
			}
		}
		buffer[i] = '\0';
		return make_string(buffer);
	}
	else if (c == '(')
	{
		return read_pair(in);
	}
	else if (isdigit(c) || (c == '-' && (isdigit(peek(in)))))
	{
		/* read a fixnum */
		if (c == '-')
		{
			sign = -1;
		}
		else
			ungetc(c, in);

		while (isdigit(c = getc(in)))
			num = (num * 10) + (c - '0');

		num *= sign;

		if (is_delimiter(c))
		{
			ungetc(c, in);
			return make_fixnum(num);
		}
		else
		{
			fprintf(stderr, "number not followed by a delimiter!");
			moduleData.running = false;
		}

	}
	else
	{
		fprintf(stderr, "unexpected '%c' \n", c);
		moduleData.running = false;
	}
	fprintf(stderr, "read illegal state \n");
	moduleData.running = false;
}

static object* eval (object* exp)
{
	return exp;
}

static void write(object* obj);

static void write_pair(object* obj)
{
	object* car_obj;
	object* cdr_obj;

	car_obj = car(obj);
	cdr_obj = cdr(obj);

	write(car_obj);

	if (cdr_obj->type == PAIR)
	{
		printf(" ");
		write_pair(cdr_obj);
	}
	else if (cdr_obj->type == EMPTY_LIST)
		return;
	else
	{
		printf(" . ");
		write(cdr_obj);
	}
}

void write(object* obj)
{
	char c;
	char *str;

	switch(obj->type)
	{
		case EMPTY_LIST:
			printf("()");
			break;

		case FIXNUM:
			printf("%ld", obj->data.fixnum.value);
			break;

		case BOOLEAN:
			printf("#%c", is_false(obj) ? 'f' : 't');
			break;

		case PAIR:
			printf("(");
			write_pair(obj);
			printf(")");
			break;

		case CHARACTER:
			c = obj->data.character.value;
			printf("#\\");

			switch(c)
			{
				case '\n':
					printf("newline");
				break;

				case ' ':
					printf("space");
					break;

				default:
					putchar(c);
			}
			break;

		case STRING:
			str = obj->data.string.value;
			putchar('"');
			while(*str != '\0')
			{
				switch(*str)
				{
					case '\n':
						printf("\\n");
						break;
					case '\\':
						printf("\\\\");
						break;
					case '"':
						printf("\\\"");
						break;
					default:
						putchar(*str);
				}
				str++;
			}
			putchar('"');
			break;

		default:
			fprintf(stderr, "cannot write, unknown type \n");
			moduleData.running = false;
			break;
	}
}

int main(int argc, char* argv[])
{

	printf("Welcome to Deea. Press ctrl-c to exit. \n");

	moduleData.running = true;

	init();

	while(moduleData.running)
	{
		printf("> ");
		write(eval(read(stdin)));
		printf("\n");
	}

	return 0;
}






















