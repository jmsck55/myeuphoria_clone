
// EuphoriaClone v0.1 (Compatible with Euphoria v3.1, v4.0.5, and v4.1)
// Front End - main routine

#include "pch.h"
#include "eu_functions.h"
#include "main.h"



int ProcessOptions()
{
	return 0;
}


void MyMain()
{
//-- Main routine
	integer argc;
	sequence argv;

	argv = eu_command_line();
	eu_print_seq(argv);

	argc = eu_length(argv);
	eu_print_int(argc);

	

	//object x;
	//x = eu_at(argv, 1);
	//eu_puts(1, x);
	//deallocate_object(x);
	
	eu_puts(1, eu_at_readonly(argv, 1));



	c_puts(stdout, "\n");


	deallocate_sequence(argv);
	

	printf("DEBUG: Program Done.\n");
	getc(stdin);
}
