// DereferencingOperatorAndPointers.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>

void PressEnterToContinue();

int main()
{
	int a;
	int *aPtr; //variable for memory address

	std::cout << "Variable 'a' is stored at address: " << &a << std::endl;
	
	__debugbreak();

	a = 23;
	aPtr = &a;

	std::cout << "The value of 'aPtr' is address" << aPtr << std::endl;
	std::cout << "The value of the address pointed to by '*aPtr' is " << *aPtr << std::endl;

	PressEnterToContinue();
    return 0;
}

void PressEnterToContinue()
{
	std::cout << "\nPress ENTER to continue..." << std::flush;
	std::cin.ignore(std::numeric_limits < std::streamsize> ::max(), '\n');
}