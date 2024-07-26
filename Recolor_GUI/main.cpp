
#include "mainwindow.h"
#include "openglwidget.h"
#include <QApplication>

#include <omp.h>
#include <opencv2/opencv.hpp>

#include <iostream>
using namespace std;

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow w;
	w.show();

	return a.exec();
}
