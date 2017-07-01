#include "writer.h"

void write_pair(object* obj)
{
	object* car_obj = car(obj);
	object* cdr_obj = cdr(obj);

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

		case SYMBOL:
			str = obj->data.string.value;
			printf("%s", str);
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
			is_running(false);
			break;
	}
}



