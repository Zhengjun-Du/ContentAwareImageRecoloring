#include "data.h"
//#include "utility.h"
#include <QFile>
#include <QDebug>
#include <algorithm>
#include <cmath>
#include <QProgressDialog>
#include <QThread>
#include <QMessagebox>
#include <QTime>
#include <omp.h>
#include <map>
#include <fstream>

using namespace std;

#define M_PI 3.14159265358979323846
#define ESP 1e-6


Data::Data()
{
	Pbr = NULL;
}

void Data::OpenImage(QString image, QString cnn)
{
	if (Pbr)
	{
		delete Pbr;
		Pbr = NULL;
	}

	img_path = image.toStdString();
	cnn_path = cnn.toStdString();
	Image Simg(img_path, cnn_path);
	SuperpixSeg Spix(img_path, 20);
	image_height = Simg.img_h;
	image_width = Simg.img_w;
	image_depth = Simg.img_c;

	if (Pbr == NULL)
		Pbr = new PaletteBasedRecoloring(Simg, Spix);

	long long totalSize = image_width * image_height * image_depth;
    if (Recolored_image != nullptr)//std::vector<double>
	{
		delete[] Recolored_image;
		Recolored_image = nullptr;
	}
	if (Original_image != nullptr)
	{
		delete[] Original_image;
		Original_image = nullptr;
	}

	Recolored_image = new double[totalSize];
	Original_image = new double[totalSize];

	for (int i = 0; i < image_height; i++) {
		for (int j = 0; j < image_width; j++) {
			int k = i * image_width + j;
			Original_image[i*image_width*image_depth + j * image_depth + 0] = Simg.features[k].r;
			Original_image[i*image_width*image_depth + j * image_depth + 1] = Simg.features[k].g;
			Original_image[i*image_width*image_depth + j * image_depth + 2] = Simg.features[k].b;
		}

	}

	memcpy(Recolored_image, Original_image, sizeof(double)*totalSize);


	emit updated();
}

void Data::ExtractPaletteWithKmenas(int num)
{
	if (Pbr == NULL) return;
	if (img_path == "") return;

	is_weight_calculated = false;
	Pbr->ExtractPaletteWithKmenas(num, img_path);

	for (int i = 0; i < image_height; i++) {
		for (int j = 0; j < image_width; j++) {
			int k = i * image_width + j;
			Original_image[i*image_width*image_depth + j * image_depth + 0] = Pbr->m_palette_Kmenas[k].r;
			Original_image[i*image_width*image_depth + j * image_depth + 1] = Pbr->m_palette_Kmenas[k].g;
			Original_image[i*image_width*image_depth + j * image_depth + 2] = Pbr->m_palette_Kmenas[k].b;
		}
	}

	emit updated();
}
//void Data::OpenPalette(string fileName)
//{
//	ifstream in(fileName);
//	double v1, v2, v3;
//	char c;
//
//	vector<vec3> rgb_ch_vertices;
//	vector<int3> rgb_ch_faces;
//	while (!in.eof()) {
//		in >> c;
//		if (c == 'v') {
//			in >> v1 >> v2 >> v3;
//			vec3 vert(v1, v2, v3);
//			rgb_ch_vertices.push_back(vert);
//		}
//		else if (c == 'f') {
//			in >> v1 >> v2 >> v3;
//			int3 face(v1 - 1, v2 - 1, v3 - 1);
//			rgb_ch_faces.push_back(face);
//		}
//		else
//			break;
//
//		c = 'x';
//	}
//
//	changed_palette.vertices = rgb_ch_vertices;
//	changed_palette.faces = rgb_ch_faces;
//
//	original_palette = changed_palette;
//
//	in.close();
//	emit updated();
//}

void Data::reset()
{
	//Weights_weights.clear();
	//Weights_weights.resize(image_width*image_height);
	is_weight_calculated = false;
}


void Data::setPaletteColor(int id, QColor c) {
	double r = qRed(c.rgb()) / 255.0;
	double g = qGreen(c.rgb()) / 255.0;
	double b = qBlue(c.rgb()) / 255.0;
	//changed_palette.vertices[id] = vec3(r, g, b);
	if (Pbr)
	{
		cv::Vec3d color(r, g, b);
		Pbr->TransferColor(color, id, &Recolored_image);

		//for (int i = 0; i < Pbr->recolored_img.size(); i++) {
		//	Recolored_image[3*i + 0] = Pbr->recolored_img[i][0];
		//	Recolored_image[3*i + 1] = Pbr->recolored_img[i][1];
		//	Recolored_image[3*i + 2] = Pbr->recolored_img[i][2];
		//}
		/*Pbr->m_palette_changed[id].r = r;
		Pbr->m_palette_changed[id].g = g;
		Pbr->m_palette_changed[id].b = b;*/
	}
}

const void Data::ComputeWeights() {

	if (Pbr == NULL) return;

	if(!Pbr->PreComputeWeights())
		return;

	is_weight_calculated = true;
	QMessageBox::information(NULL, "Info", "Weight finished", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
}

void Data::Recolor() {
	/*if (!is_weight_calculated)
		return;

	QTime time1;
	time1.start();*/

//#pragma omp parallel for num_threads(32)
//	for (int i = 0; i < image_width * image_height * image_depth; i += image_depth)
//	{
//		int frame_pix_offset = image_width * image_height * image_depth;
//		vec3 x(
//			Original_image[i + 0],
//			Original_image[i + 1],
//			Original_image[i + 2]
//		);
//
//		vec3 y = RecolorSinglePixel(i / image_depth, x);
//
//		Recolored_image[i + 0] = y[0];
//		Recolored_image[i + 1] = y[1];
//		Recolored_image[i + 2] = y[2];
//	}

	//qDebug() << "Recolor time: " << time1.elapsed() / 1000.0 << "s";
	emit updated();
}

void Data::ExportOriginalImage(string path) {
	if (!is_weight_calculated)
		return;

	QTime time;
	time.start();

	int k = 0;

	QSize size(image_height, image_width);
	QImage image(size, QImage::Format_ARGB32);

#pragma omp parallel for num_threads(32)
	for (int i = 0; i < image_width * image_height * image_depth; i += image_depth)
	{
		vec3 x(
			Original_image[i + 0],
			Original_image[i + 1],
			Original_image[i + 2]
		);


		int row = i / (image_height * image_depth);
		int col = (i - row*image_height * image_depth) / image_depth;
		image.setPixel(col, row, qRgb(x[0] * 255, x[1] * 255, x[2] * 255));

	}
	image.save((path + "/original.png").c_str(), "PNG", 100);
}

void Data::ExportRecoloredImage(string path) {


	uchar *data = new uchar[image_width * image_height * image_depth];

	#pragma omp parallel for num_threads(32)
	for (int i = 0; i < image_width * image_height * image_depth; i++)
	{
		int x = static_cast<int>(Recolored_image[i] * 255);
		if (x > 255) x = 255;
		if (x < 0) x = 0;

		data[i] = static_cast<uchar>(x);
	}

	cout << endl;
	for (int i = 0; i < 3; i++)
		cout << (int)data[i * 3] << "," << (int)data[i * 3 + 1] << "," << (int)data[i * 3 + 2] << endl;

	QImage image(data, image_width, image_height, image_width * 3, QImage::Format_RGB888);
	image.save((path + "/recolored.png").c_str(), "PNG", 100);
}

void Data::ExportPs(string path) { //ÐÂ¼ÓµÄ

#pragma omp parallel for num_threads(32)
	for (int j = 0; j < Pbr->m_palette.size(); j++) {
		uchar* data = new uchar[image_width * image_height * image_depth];
		for (int i = 0; i < image_width * image_height * image_depth; i++)
		{
			int x = static_cast<int>(Pbr->m_weights[i / 3][j] * 255);
			if (x > 255) x = 255;
			if (x < 0) x = 0;
			data[i] = static_cast<uchar>(x);
		}
		QImage image(data, image_width, image_height, image_width * 3, QImage::Format_RGB888);
		image.save((path + "/Layer" + char(j+'0') + ".png").c_str(), "PNG", 100);
	}
}



void Data::ResetPaletteColor(int id) {
	Pbr->m_palette_changed[id].r = Pbr->m_palette[id].r;
	Pbr->m_palette_changed[id].g = Pbr->m_palette[id].g;
	Pbr->m_palette_changed[id].b = Pbr->m_palette[id].b;

	cv::Vec3d color(Pbr->m_palette[id].r, Pbr->m_palette[id].g, Pbr->m_palette[id].b);
	Pbr->TransferColor(color, id, &Recolored_image);

	Recolor();
	emit updated();
}

void Data::ResetAllPaletteColors() {
	if (Pbr == NULL) return;

	for (int i = 0; i < Pbr->m_palette_changed.size(); i++)
	{
		Pbr->m_palette_changed[i].r = Pbr->m_palette[i].r;
		Pbr->m_palette_changed[i].g = Pbr->m_palette[i].g;
		Pbr->m_palette_changed[i].b = Pbr->m_palette[i].b;
		cv::Vec3d color(Pbr->m_palette[i].r, Pbr->m_palette[i].g, Pbr->m_palette[i].b);
		Pbr->TransferColor(color, i, &Recolored_image);
	}

	Recolor();
	emit updated();
}

void Data::ImportChangedPalette(string path) {
	if (Pbr == NULL)
		return;

	ifstream ifs(path);
	int n; ifs >> n;
	for (int i = 0; i < n; i++) {
		ifs >> Pbr->m_palette_changed[i].r >> Pbr->m_palette_changed[i].g
			>> Pbr->m_palette_changed[i].b;
	}
	emit updated();
}


void Data::ExportChangedPalette(string path) {
	if (Pbr == NULL)
		return;

	ofstream ofs(path);
	ofs << Pbr->m_palette_changed.size() << endl;
	for (int i = 0; i < Pbr->m_palette_changed.size(); i++) {

		ofs << Pbr->m_palette_changed[i].r << " " <<
			Pbr->m_palette_changed[i].g << " " <<
			Pbr->m_palette_changed[i].b << " " << endl;
	}
}
