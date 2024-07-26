#ifndef DATA_H
#define DATA_H

#include<QString>
#include<QObject>
#include<vector>
#include <QThread>
#include"utility.h"
#include "vec3.h"
#include <string>
#include "PaletteExtraction/Utility.h"
#include "PaletteExtraction/SuperpixSeg.h"
#include "PaletteExtraction/PaletteBasedRecoloring.h"
using namespace std;

class Data : public QObject
{
    Q_OBJECT

public:
    Data();
	void OpenImage(QString image, QString cnn);
	void ExtractPaletteWithKmenas(int _num);
    //void OpenPalette(string fileName);
    void reset();

	double *GetImage(bool isAfter = true) const { return isAfter ? Recolored_image : Original_image; }
	vector<Feature> GetChangedPalette() { return Pbr ? Pbr->m_palette_changed : vecEmpty; }
	vector<Feature> GetOriginalPalette() { return Pbr ? Pbr->m_palette : vecEmpty; }
	vector<Feature> GetPalette() 
	{
		if (Pbr == NULL)
			return vecEmpty;
		return is_weight_calculated ? Pbr->m_palette_changed : Pbr->m_palette; 
	}

	const void ComputeWeights();

	void Recolor();
	
    int GetFrameWidth() const { return image_width; }
    int GetFrameHeight() const{ return image_height; }
    int GetFrameDepth() const{ return image_depth; }

	void setPaletteColor(int id, QColor c);
	void ResetPaletteColor(int id);
	void ResetAllPaletteColors();

	void ExportOriginalImage(string path);
	void ExportRecoloredImage(string path);
	void ExportChangedPalette(string path);
	void ImportChangedPalette(string path);
	void ExportPs(string path);//ÐÂ¼ÓµÄ

   
public slots:
signals:
    void updated();

private:
	double* Recolored_image = nullptr;
	double* Original_image = nullptr;
	
	//vector<vector<float> > Weights_weights;
	bool is_weight_calculated;

	PaletteBasedRecoloring* Pbr;
	vector<Feature> vecEmpty;
	std::string img_path;
	std::string cnn_path;
	int image_width = 0;
	int image_height = 0;
	int image_depth = 0;
};

#endif // DATA_H