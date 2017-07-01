#include "reader.h"

object* read_character(FILE* in)
{
	int c;

	c = getc(in);

	switch(c)
	{
		case EOF:
			fprintf(stderr, "incomplete character literal \n");
			is_running(false);
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
			is_running(false);
		}

		cdr_obj = read(in);
		eat_whitespace(in);
		c = getc(in);

		if (c != ')')
		{
			fprintf(stderr, "missing right paren\n");
			is_running(false);
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
				is_running(false);
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
	/* read a symbol */
	else if ( (is_initial(c) || c == '+' || c == '-')
			//&& is_delimiter(peek(in))
			)
	{
		i = 0;
		while (is_initial(c) || isdigit(c) || c == '+' || c == '-')
		{
			if (i < BUFFER_MAX - 1)
				buffer[i++] = c;
			else
			{
				fprintf(stderr, "symbol too long. Maximum length is %d \n", BUFFER_MAX);
				is_running(false);
			}
			c = getc(in);
		}

		if (is_delimiter(c))
		{
			buffer[i] = '\0';
			ungetc(c, in);
			return make_symbol(buffer);
		}
		else
		{
			fprintf(stderr, "symbol not followed by a delimitator. Found '%c'\n", c);
			is_running(false);
		}

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
			is_running(false);
		}

	}
	else
	{
		fprintf(stderr, "unexpected '%c' \n", c);
		is_running(false);
	}
	fprintf(stderr, "read illegal state \n");
	is_running(false);
}
