#include <iostream>
#include <QApplication>
#include <QLabel>
#include "example_folder1/dummy11.h"

int main(int argCount, char** argValues) {
	QApplication app(argCount, argValues);
	QLabel* label = new QLabel("Yo wassup, welcome to Electronics Components Invetory Manager");

	label->setWindowTitle("Electronics Components Inventory Manager");
	label->resize(600, 400);
	label->show();

	dummy11(); // just to test that the function is callable from main.cpp, and that the header is correctly included and linked
	return app.exec();
}
