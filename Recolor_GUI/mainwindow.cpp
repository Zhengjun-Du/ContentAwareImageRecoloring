#include<QWidget>
#include<QPushButton>
#include<QVBoxLayout>
#include<QLabel>
#include<QDockWidget>
#include<QTextEdit>
#include<QLineEdit>
#include<QListWidget>
#include<QFileDialog>
#include<QCheckBox>
#include <QGroupBox>
#include "mainwindow.h"
#include "imagewidget.h"
#include "openglwidget.h"
#include "rgbwidget.h"
#include "paletteviewwidget.h"
#include "Qt-Color-Widgets\include\QtColorWidgets\color_dialog.hpp"
#include <QColorDialog>
#include "data.h"
using namespace color_widgets;

MainWindow::MainWindow(QWidget *parent) :QMainWindow(parent){
    setWindowTitle(title);
	setGeometry(100, 100, 520, 520);
    QWidget *mainWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout();
	
	data = new Data();

	//top main menu==============================================================================================
    QHBoxLayout *FirstRowLayout = new QHBoxLayout();
    QPushButton *OpenImageAndPaletteBtn = new QPushButton("Open Image && Semantic map");
    QPushButton *CalcWeightsBtn = new QPushButton("Calc. Weight ");
	QPushButton *RgbCalcPaletteButton = new QPushButton("Extract Palette");
	EditNum = new QLineEdit();
	EditNum->setValidator(new QIntValidator(EditNum));
	EditNum->setMaximumWidth(50);
	EditNum->setText("6");

    FirstRowLayout->addWidget(OpenImageAndPaletteBtn);
	FirstRowLayout->addWidget(RgbCalcPaletteButton);
	FirstRowLayout->addWidget(EditNum);
	FirstRowLayout->addWidget(CalcWeightsBtn);
	mainLayout->addLayout(FirstRowLayout);

	//original image and recolored image=========================================================================
    ImageWidget *Recolored_image = new ImageWidget(true);
	Recolored_image->setData(data);

    ImageWidget *Original_image = new ImageWidget(false);
	Original_image->setData(data);

    imageBeforeDockWidget = new QDockWidget();
    imageBeforeDockWidget->setWidget(Original_image);
    imageBeforeDockWidget->setWindowTitle("Original Image");
    addDockWidget(Qt::TopDockWidgetArea, imageBeforeDockWidget);
    imageBeforeDockWidget->setFloating(true);
    imageBeforeDockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
	imageBeforeDockWidget->setGeometry(760, 100, 400, 400);
	imageBeforeDockWidget->hide();

    imageAfterDockWidget = new QDockWidget();
    imageAfterDockWidget->setWidget(Recolored_image);
    imageAfterDockWidget->setWindowTitle("Recolored Image");
    imageAfterDockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::TopDockWidgetArea, imageAfterDockWidget);
    imageAfterDockWidget->setFloating(true);
	imageAfterDockWidget->setGeometry(760, 100, 400, 400);
	imageAfterDockWidget->hide();
	//original image and recolored image=========================================================================

	//import and export =========================================================================================
	QHBoxLayout *secondRowLayout = new QHBoxLayout();
	QPushButton *exportImageeBtn = new QPushButton("Export Image");
	QPushButton* exportpsBtn = new QPushButton("Export Layers");
	QPushButton *importPaletteButton = new QPushButton("Import Palette");
	QPushButton *exportPaletteButton = new QPushButton("Export Palette");
	secondRowLayout->addWidget(exportImageeBtn);
	secondRowLayout->addWidget(exportpsBtn);
	secondRowLayout->addWidget(importPaletteButton);
	secondRowLayout->addWidget(exportPaletteButton);

	QGroupBox *groupBox = new QGroupBox(tr("Import and Export"));
	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->addLayout(secondRowLayout);
	groupBox->setLayout(vbox);

    QDockWidget *dockPaletteWidget = new QDockWidget();
    PaletteViewWidget *paletteWidget = new PaletteViewWidget();
    paletteWidget->setMinimumSize(500, 150);
    paletteWidget->setData(data);

    dockPaletteWidget->setWidget(paletteWidget);
    dockPaletteWidget->setWindowTitle("Palette");
    addDockWidget(Qt::RightDockWidgetArea, dockPaletteWidget);
    dockPaletteWidget->setFloating(true);
	dockPaletteWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);

	color_widgets::ColorDialog* dialog = new color_widgets::ColorDialog();
	dialog->setWindowTitle("Color picker");

	QDockWidget *dockColorWidget = new QDockWidget();

	dockColorWidget->setWidget(dialog);
	dockColorWidget->setWindowTitle("Color picker");
	addDockWidget(Qt::RightDockWidgetArea, dockColorWidget);
	dockColorWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);

	mainLayout->addWidget(groupBox);
	mainLayout->addWidget(dockPaletteWidget);
	mainLayout->addWidget(dockColorWidget);
	
	mainWidget->setLayout(mainLayout);
	this->setCentralWidget(mainWidget);

    connect(OpenImageAndPaletteBtn, &QPushButton::clicked, [=](){this->openFile(true);});
    connect(CalcWeightsBtn, &QPushButton::clicked, data, &Data::ComputeWeights);
	connect(dialog, &ColorDialog::colorChanged, paletteWidget, &PaletteViewWidget::getColor);
	connect(paletteWidget, &PaletteViewWidget::setColor, dialog, &ColorDialog::setColor);
	connect(RgbCalcPaletteButton, &QPushButton::clicked, [=]() {
		int num = EditNum->text().toInt();
		if (num < 1){
			num = 6;
			EditNum->setText(QString::number(num));
		}
		if (data)
			data->ExtractPaletteWithKmenas(num);
	});

	connect(exportImageeBtn, &QPushButton::clicked, [=]() { this->exportImage(); });
	connect(exportpsBtn, &QPushButton::clicked, [=]() { this->exportps(); });//新加的
	connect(importPaletteButton, &QPushButton::clicked, [=]() { this->importPalette(); });
	connect(exportPaletteButton, &QPushButton::clicked, [=]() { this->exportPalette(); });
}

MainWindow::~MainWindow()
{
}

// open image 
void MainWindow::openFile(bool merge){
    if(data == nullptr) return;

    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("*.jpg *.png"));
    dialog.setViewMode(QFileDialog::Detail);

	QString image_file, data_file;
    if (dialog.exec()){
        QStringList fileName = dialog.selectedFiles();

        for(auto s : fileName){
			image_file = s;
        }
    }
    else{
        return;
    }

    dialog.setNameFilter("*.data");
    dialog.setFileMode(merge ? QFileDialog::ExistingFiles : QFileDialog::ExistingFile);

    if (dialog.exec()){
        QStringList fileName = dialog.selectedFiles();

        fileName.sort();
        data->reset();
        if(mergeStepSlider != nullptr)
            mergeStepSlider->setMaximum(fileName.size() - 1);

        int i = 0;
        for(auto s : fileName){
			data_file = s;
            i++;
        }
		data->OpenImage(image_file, data_file);
    }
	imageBeforeDockWidget->show();
	imageAfterDockWidget->show();
}

void MainWindow::exportImage() {
	if (data == nullptr) return;

	QFileDialog fileDialog(this);
	fileDialog.setFileMode(QFileDialog::DirectoryOnly);
	QStringList dirName;
	if (fileDialog.exec() == QDialog::Accepted){
		dirName = fileDialog.selectedFiles();
		string dirNamestr = dirName[0].toStdString();

		string recolor_dir = dirNamestr;
		data->ExportRecoloredImage(recolor_dir);
	}
}

void MainWindow::exportps() {
	if (data == nullptr) return;

	QFileDialog fileDialog(this);
	fileDialog.setFileMode(QFileDialog::DirectoryOnly);
	QStringList dirName;
	if (fileDialog.exec() == QDialog::Accepted)
	{
		dirName = fileDialog.selectedFiles();
		string dirNamestr = dirName[0].toStdString();

		string recolor_dir = dirNamestr;
		data->ExportPs(recolor_dir);
	}
}

void MainWindow::importPalette() {
	if (data == nullptr) return;

	QFileDialog fileDialog(this);
	fileDialog.setFileMode(QFileDialog::QFileDialog::ExistingFile);
	fileDialog.setNameFilter(tr("*.txt"));
	fileDialog.setViewMode(QFileDialog::Detail);

	QStringList dirName;
	if (fileDialog.exec() == QDialog::Accepted){
		dirName = fileDialog.selectedFiles();
		string dirNamestr = dirName[0].toStdString();
		data->ImportChangedPalette(dirNamestr);
	}
}

void MainWindow::exportPalette() {
	if (data == nullptr) return;

	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Data"), ".", tr("txt File (*.txt)"));
	data->ExportChangedPalette(fileName.toStdString());
}
