#include "Application.hpp"

#include <QApplication>
#include <QLabel>

int execECIMApplication(int argCount, char** argValues) {
	QApplication app(argCount, argValues);
	QLabel* label = new QLabel("Yo wassup, welcome to Electronics Components Invetory Manager");

	label->setWindowTitle("Electronics Components Inventory Manager");
	label->resize(600, 400);
	label->show();

	return app.exec();
}
