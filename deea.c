#include "deea.h"
#include "writer.h"
#include "eval.h"
#include "reader.h"
#include "helper.h"

void init(void)
{
	o_is_false = allocate_object();
	o_is_false->type = BOOLEAN;
	o_is_false->data.boolean.value = false;

	o_is_true = allocate_object();
	o_is_true->type = BOOLEAN;
	o_is_true->data.boolean.value = true;

	o_empty_list = allocate_object();
	o_empty_list->type = EMPTY_LIST;

	o_symbol_table = o_empty_list;

	o_quote_symbol = make_symbol("quote");
	o_ok_symbol = make_symbol("ok");
	o_define_symbol = make_symbol("define");
	o_set_symbol = make_symbol("set!");

	o_the_empty_environment = o_empty_list;
	o_the_global_environment = setup_environment();

}

int main(int argc, char* argv[])
{

	printf("Welcome to Deea. Press ctrl-c to exit. \n");

	set_running(true);

	init();

	while(is_running())
	{
		printf("> ");
		write(eval(read(stdin), o_the_global_environment));
		printf("\n");
	}

	return 0;
}

