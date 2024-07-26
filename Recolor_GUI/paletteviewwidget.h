#ifndef PALETTEVIEWWIDGET_H
#define PALETTEVIEWWIDGET_H

#include<openglwidget.h>
#include <QMenu>
#include <QDateTime>

class PaletteViewWidget : public OpenGLWidget
{
Q_OBJECT
public:
    explicit PaletteViewWidget(QWidget *parent = 0);

protected:
    void paintGL() Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) override;

public Q_SLOTS:
	void getColor(QColor c);
	void setTime(int t_);

	void ResetPaletteColor();
	void ResetAllPaletteColors();
	
Q_SIGNALS:
	void setColor(QColor c);

private:
    bool blink = false;
	double scale = 1.95;
	int selected_vid = -1;
	double aspect = 1.0;
	int time = 0;

	vector<vec3> palette_pos;
	QMenu *pMenu;
	QDateTime curTime = QDateTime::currentDateTime();
};

#endif // PALETTEVIEWWIDGET_H
