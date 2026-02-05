#include <iostream>
#include "example_folder1/dummy11.h"

int main() {
	std::cout << "Electronics Inventory - hello" << std::endl;
	dummy11(); // just to test that the function is callable from main.cpp, and that the header is correctly included and linked
	return 0;
}
