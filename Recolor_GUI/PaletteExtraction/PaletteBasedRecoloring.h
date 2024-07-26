#pragma once

#include "Utility.h"
#include "SuperpixSeg.h"
#include "ModifiedKmeans.h"
#include <vector>
#include <algorithm>
using namespace std;

class PaletteBasedRecoloring {
public:
	Image m_simg;
	SuperpixSeg m_spix;
	vector<Feature> m_palette;
	vector<Feature> m_palette_changed;
	vector<Feature> m_palette_Kmenas;
	vector<Feature> delta_palette;

	vector<vector<double>> m_weights;
	vector<cv::Vec3d> recolored_img;
	double m_sigma_rgb, m_sigma_xy, m_sigma_semantic;
	
public:
	PaletteBasedRecoloring() {}

	PaletteBasedRecoloring(Image& si,  SuperpixSeg& spix){
		m_simg = si;
		m_spix = spix;
	}

	void ExtractPaletteWithKmenas(int K, string img_path) {
		vector<spixCenter> spixCenters = m_spix.GetSuperpixCenters();
		vector<cv::Vec2i> spixCenterPos(spixCenters.size());
		vector<int> pixCntInSpix(spixCenters.size());
		for (int i = 0; i < spixCenters.size(); i++) {
			spixCenterPos[i] = cv::Vec2i(spixCenters[i].y, spixCenters[i].x); //row, col
			pixCntInSpix[i] = spixCenters[i].pix_cnt;
		}
		vector<Feature> features = m_simg.GetFeaturesWith(spixCenterPos);

		ModifiedKmeans Mk(features, pixCntInSpix, K);
		m_palette = Mk.GetClusterCenters();
		m_palette_changed = Mk.GetClusterCenters();
		//delta_palette = Mk.GetClusterCenters();

		m_palette_Kmenas = Mk.DrawKmenasResultEx(img_path);
		//Mk.VisualizeKmenas(img_path);
	}

	void ExtractPaletteWithMeanShift() {

	}

	void ComputeMeanOfPalette() {
		int tot = 0;
		m_sigma_rgb = 0, m_sigma_xy = 0, m_sigma_semantic = 0;
		for (int i = 0; i < m_palette.size(); i++) {
			for (int j = i + 1; j < m_palette.size(); j++) {
				m_sigma_rgb += GetColorDistanceOf(m_palette[i], m_palette[j]);
				m_sigma_xy += GetPositionDistanceOf(m_palette[i], m_palette[j]);
				m_sigma_semantic += GetSemanticDistanceOf(m_palette[i], m_palette[j]);
				tot++;
			}
		}
		m_sigma_rgb /= tot;
		m_sigma_xy /= tot;
		m_sigma_semantic /= tot;
	}

	double GetPhiOf(Feature f1, Feature f2) {
		double dis_rgb = GetColorDistanceOf(f1, f2);
		double dis_xy = GetPositionDistanceOf(f1, f2);
		double dis_semantic = GetSemanticDistanceOf(f1, f2);
		double G_rgb = exp(-dis_rgb * dis_rgb / (2 * m_sigma_rgb * m_sigma_rgb));
		double G_xy = exp(-dis_xy * dis_xy / (2 * m_sigma_xy * m_sigma_xy));
		double G_semantic = exp(-dis_semantic * dis_semantic / (2 * m_sigma_semantic * m_sigma_semantic));
		
		//G_xy = 1;
		return G_rgb * G_xy * G_semantic;
	}

	double GetPhiOf(double dis_rgb, double dis_xy, double dis_semantic) {
		double G_rgb = exp(-dis_rgb * dis_rgb / (2 * m_sigma_rgb * m_sigma_rgb));
		double G_xy = exp(-dis_xy * dis_xy / (2 * m_sigma_xy * m_sigma_xy));
		double G_semantic = exp(-dis_semantic * dis_semantic / (2 * m_sigma_semantic * m_sigma_semantic));
		//G_xy = 1;
		return G_rgb * G_xy * G_semantic;
	}

	cv::Mat PreComputeLamda() {
		cv::Mat phi_mat = cv::Mat_<double>(m_palette.size(), m_palette.size());
		for (int i = 0; i < m_palette.size(); i++) {
			for (int j = 0; j < m_palette.size(); j++) {
				double phi = GetPhiOf(m_palette[i], m_palette[j]);
				phi_mat.at<double>(i, j) = phi;
			}
		}

		cv::Mat lambda_mat;
		invert(phi_mat, lambda_mat);

		cout << "phi_mat" << endl;
		cout << phi_mat << endl << endl;

		cout << "lambda_mat" << endl;
		cout << lambda_mat << endl << endl;

		return lambda_mat;
	}

	bool PreComputeWeights() {
		int pn = m_palette.size();
		if (pn == 0) return false;
		ComputeMeanOfPalette();

		
		m_weights = vector<vector<double>>(m_simg.img_h * m_simg.img_w);
		
		cv::Mat lambda_mat = PreComputeLamda();
		
#pragma omp parallel for
		for (int k = 0; k < m_simg.features.size(); k++){
			Feature f = m_simg.features[k];

			vector<double> dis_rgb(pn);
			vector<double> dis_xy(pn);
			vector<double> dis_semantic(pn);

			for (int i = 0; i < pn; i++) {
				dis_rgb[i] = GetColorDistanceOf(f, m_palette[i]);
				dis_xy[i] = GetPositionDistanceOf(f, m_palette[i]);
				dis_semantic[i] = GetSemanticDistanceOf(f, m_palette[i]);
			}

			vector<double> w_f(pn, 0);
			for (int i = 0; i < pn; i++) {
				for (int j = 0; j < pn; j++)
					w_f[i] += lambda_mat.at<double>(i, j) * GetPhiOf(dis_rgb[j], dis_xy[j], dis_semantic[j]);
			}

			double w_sum = 0;
			for (int i = 0; i < pn; i++) {
				if (w_f[i] < 0) w_f[i] = 0;
				w_sum += w_f[i];
			}

			for (int i = 0; i < pn; i++) {
				w_f[i] /= w_sum;
			}
			m_weights[k] = w_f;
		}

		return true;
	}

	void TransferColor(vector<Feature> modified_palette) {
		vector<Feature> delta_palette = modified_palette;
		for (int i = 0; i < m_palette.size(); i++)
			delta_palette[i] -= m_palette[i];

		recolored_img.clear();
		recolored_img.resize(m_simg.img_h * m_simg.img_w);
#pragma omp parallel for
		for (int i = 0; i < recolored_img.size(); i++) {
			recolored_img[i] = m_simg.features[i].GetColor();
			for (int j = 0; j < m_palette.size(); j++)
				recolored_img[i] += m_weights[i][j] * delta_palette[j].GetColor();
		}

		cv::Mat recolored(m_simg.img_h, m_simg.img_w, CV_8UC3, cv::Scalar(255, 255, 255));
		for (int i = 0; i < recolored_img.size(); i++) {
			int r = i / m_simg.img_w;
			int c = i % m_simg.img_w;
			recolored.at<cv::Vec3b>(r, c)[2] = recolored_img[i][0] * 255;
			recolored.at<cv::Vec3b>(r, c)[1] = recolored_img[i][1] * 255;
			recolored.at<cv::Vec3b>(r, c)[0] = recolored_img[i][2] * 255;	
		}

		cv::imwrite("recolored.jpg", recolored);
		cv::waitKey(0);
	}

	void TransferColor(cv::Vec3d c, int i, double** Recolored_image) {
		if (m_weights.size() == 0) return;

		//vector<Feature> delta_palette(m_palette.size());
		if (delta_palette.size() != m_palette.size())
			delta_palette.resize(m_palette.size());
		m_palette_changed[i].r = c[0];
		m_palette_changed[i].g = c[1];
		m_palette_changed[i].b = c[2];
		delta_palette[i].r = c[0] - m_palette[i].r;
		delta_palette[i].g = c[1] - m_palette[i].g;
		delta_palette[i].b = c[2] - m_palette[i].b;
		//recolored_img.clear();
		int size = m_simg.img_h * m_simg.img_w;
		if(recolored_img.size() != size)
			recolored_img.resize(size);

		for (int i = 0; i < recolored_img.size(); i++) {
			recolored_img[i] = m_simg.features[i].GetColor();

			for (int j = 0; j < m_palette.size(); j++)
				recolored_img[i] += m_weights[i][j] * delta_palette[j].GetColor();

			(*Recolored_image)[3 * i + 0] = recolored_img[i][0];
			(*Recolored_image)[3 * i + 1] = recolored_img[i][1];
			(*Recolored_image)[3 * i + 2] = recolored_img[i][2];
		}

		/*cv::Mat recolored(m_simg.img_h, m_simg.img_w, CV_8UC3, cv::Scalar(255, 255, 255));
		for (int i = 0; i < recolored_img.size(); i++) {
			int r = i / m_simg.img_w;
			int c = i % m_simg.img_w;
			recolored.at<cv::Vec3b>(r, c)[2] = min(max(recolored_img[i][0], 0.0), 1.0) * 255;
			recolored.at<cv::Vec3b>(r, c)[1] = min(max(recolored_img[i][1], 0.0), 1.0) * 255;
			recolored.at<cv::Vec3b>(r, c)[0] = min(max(recolored_img[i][2], 0.0), 1.0) * 255;
		}*/

		//cv::imwrite(save_path, recolored);
		//cv::waitKey(0);
	}

	cv::Vec3d GetPaletteColor(int i) {
		return cv::Vec3d(m_palette[i].r, m_palette[i].g, m_palette[i].b);
	}
};