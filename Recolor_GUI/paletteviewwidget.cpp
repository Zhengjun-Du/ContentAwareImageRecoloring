#include "paletteviewwidget.h"

#include<QTimer>
#include<QDebug>
#include<cmath>
#include<QMouseEvent>
#include<QCryptographicHash>
#include <QColorDialog>
#include <QMenu>
#include "qrgb.h"
#include "math.h"
#include <algorithm>

#define MAX_CHAR       128
#include "Qt-Color-Widgets\include\QtColorWidgets\color_dialog.hpp"
using namespace std;
using namespace color_widgets;

// Widget shows a palette-time view

PaletteViewWidget::PaletteViewWidget(QWidget *parent) : OpenGLWidget(parent)
{
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [=](){ blink = !blink; update(); });
    timer->start(800);

	pMenu = new QMenu(this);
	QAction *pResetTask = new QAction(tr("reset this color"), this);
	QAction *pResetAllTask = new QAction(tr("reset all color"), this);
	pMenu->addAction(pResetTask);
	pMenu->addAction(pResetAllTask);

	connect(pResetTask, SIGNAL(triggered()), this, SLOT(ResetPaletteColor()));
	connect(pResetAllTask, SIGNAL(triggered()), this, SLOT(ResetAllPaletteColors()));
}

void PaletteViewWidget::setTime(int t) {
	time = t;
	if (selected_vid != -1 ) {
		data->Recolor();
	}
}

void PaletteViewWidget::getColor(QColor c) {
	//int r = qRed(c.rgb());
	//int g = qGreen(c.rgb());
	//int b = qBlue(c.rgb());
	
	if (selected_vid != -1) {
		if (curTime.msecsTo(QDateTime::currentDateTime()) < 500)
			return;
		curTime = QDateTime::currentDateTime();
		//if(curTime.msecsTo(QDateTime::currentDateTime() );
		data->setPaletteColor(selected_vid, c);
		update();
		data->Recolor();
	}
}


void drawString(const char* str) {
	static int isFirstCall = 1;
	static GLuint lists;

	if (isFirstCall) { // 如果是第一次调用，执行初始化
						// 为每一个ASCII字符产生一个显示列表
		isFirstCall = 0;

		// 申请MAX_CHAR个连续的显示列表编号
		lists = glGenLists(MAX_CHAR);

		// 把每个字符的绘制命令都装到对应的显示列表中
		wglUseFontBitmaps(wglGetCurrentDC(), 0, MAX_CHAR, lists);
	}
	// 调用每个字符对应的显示列表，绘制每个字符
	for (; *str != '\0'; ++str)
		glCallList(lists + *str);
}

void PaletteViewWidget::paintGL()
{
    OpenGLWidget::paintGL();

    glScalef(scale, scale, scale);

	int w = width(), h = height();
	double aspect = w*1.0 / h;

	//绘制白色矩形框
	/*
	glBegin(GL_LINES);

	for (int p = 0; p <= 1; p++)
	{
		double q = p - .5;
		glColor3f(1., 1., 1.);
		glVertex3f(q, -.5, 0.);
		glColor3f(1., 1., 1.);
		glVertex3f(q, .5, 0.);
		glColor3f(1., 1., 1.);
		glVertex3f(-.5, q, 0.);
		glColor3f(1., 1., 1.);
		glVertex3f(.5, q, 0.);
	}
	glEnd();
	*/

	vector<Feature> ori_poly = data->GetOriginalPalette();
	vector<Feature> cur_poly = data->GetChangedPalette();
	

	float space = 0.02;
	float recw = 0.06;
	float x = -0.5, y = 0.45, y1 = 0.1;
	palette_pos.clear();
	
    for (auto i = 0; i < ori_poly.size(); i++)
	{
		Feature overt = ori_poly[i];
        float o_r = overt.r;
        float og = overt.g;
        float ob = overt.b;
        glColor3f(o_r , og, ob);
		glRectf(x, y, x + recw, y - recw*aspect);
		
		Feature cvert = cur_poly[i];
		float cr = cvert.r, cg = cvert.g, cb = cvert.b;

		if (selected_vid == i) {
			double ex = 0.006;
			glColor3f(0.8, 0, 0);
			//glRectf(x- ex, y1+ ex*aspect, x + recw+ ex, y1 - recw*aspect- ex*aspect);
			glRectf(x, y1-recw*aspect - 0.01*aspect, x + recw, y1 - recw*aspect - 0.02*aspect);

		}

		glColor3f(cr , cg, cb);
		glRectf(x, y1, x + recw, y1 - recw*aspect);

		glColor3f(0.0f, 0.0f, 0.0f);
		glRasterPos2f(x + recw / 3, -0.25);
		drawString(to_string(i).c_str());

		palette_pos.push_back(vec3(x + recw / 2, y1 - recw*aspect / 2, 0));

		x += recw + space;
	}
}

void PaletteViewWidget::mousePressEvent(QMouseEvent *ev)
{
	if (ev->button() == Qt::LeftButton) {
		//1. 坐标系变换，将屏幕坐标系 -> opengl坐标系
		int w = width(), h = height();
		double half_w = w / 2.0, half_h = h / 2.0;
		double new_x = (ev->x() - half_w) / half_w / scale;
		double new_y = -(ev->y() - half_h) / half_h / scale;

		vector<Feature> cur_poly = data->GetChangedPalette();

		//2. 找到离鼠标点击最近的点
		double aspect = w*1.0 / h;
		float recw = 0.07;
		float rech = recw * aspect;

		for (int i = 0; i < cur_poly.size(); i++)
		{
			double x = palette_pos[i][0];
			double y = palette_pos[i][1];

			double dis1 = fabs(x - new_x);
			double dis2 = fabs(y - new_y);

			if (dis1 < recw / 2 && dis2 < rech / 2) {
				selected_vid = i;

				double r = cur_poly[i].r;
				double g = cur_poly[i].g;
				double b = cur_poly[i].b;

				int R = r * 255, G = g * 255, B = b * 255;
				QColor c;
				c.setRed(R);
				c.setGreen(G);
				c.setBlue(B);
				Q_EMIT setColor(c);
				break;
			}
		}
	}
	else if (ev->button() == Qt::RightButton) {
		pMenu->exec(cursor().pos());
	}
}

void PaletteViewWidget::ResetPaletteColor() {
	if (selected_vid != -1) {
		data->ResetPaletteColor(selected_vid);
		emit update();
	}
}
void PaletteViewWidget::ResetAllPaletteColors() {
	if (selected_vid != -1) {
		data->ResetAllPaletteColors();
		emit update();
	}
}
