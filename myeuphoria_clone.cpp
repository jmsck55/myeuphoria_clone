//
// euphoria_replacement.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

extern "C" {
#include "pch.h"
#include "eu_functions.h"
#include "main.h"
}

int main(int argc, char* argv[])
{
/*	int i;
	char* s;
	for (i = 1; i < 200; i++)
	{
		s = strerror(i);
		printf("errno %i: %s\n", i, s);
	}
*/
	int i;
	printf("DEBUG: Command line args from C++, %i\n", argc);
	for (i = 0; i < argc; i++)
	{
		printf("%s\n", argv[i]);
	}
	printf("DEBUG: Your program begins after this line:\n");
	printf("-------------------------------------------\n");

	eu_init(argc, argv);
	MyMain();
	return 0;
}


// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
