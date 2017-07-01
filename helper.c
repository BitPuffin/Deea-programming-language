#include "helper.h"


int peek(FILE* in)
{
	int c;

	c = getc(in);
	ungetc(c, in);

	return c;
}

void eat_string(FILE* in, char* str)
{
	int c;

	while(*str != '\0')
	{
		c = getc(in);
		if (c != *str)
		{
			fprintf(stderr, "unexpected character '%c'\n", c);
			set_running(false);
		}
		str++;
	}
}

void eat_whitespace(FILE* in)
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

void peek_expected_delimiter(FILE* in)
{
	if (!is_delimiter(peek(in)))
	{
		fprintf(stderr, "character not followed by delimiter \n");
		set_running(false);
	}
}


