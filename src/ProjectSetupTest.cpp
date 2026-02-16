#include "ProjectSetupTest.h"

/*
 * Note:
 *
 * This file is only intended to test project setup. This file MUST be removed
 * when development of the application begins.
 */

 #include <iostream>

 #include <QApplication>
 #include <QLabel>

int RunTestQtApp(int argCount, char **argValues) {
	std::cout << "Running Test Qt App" << std::endl;

	QApplication app(argCount, argValues);
	QLabel* label = new QLabel("Yo wassup, welcome to Electronics Components Invetory Manager");

	label->setWindowTitle("Electronics Components Inventory Manager");
	label->resize(600, 400);
	label->show();

	return app.exec();
}
