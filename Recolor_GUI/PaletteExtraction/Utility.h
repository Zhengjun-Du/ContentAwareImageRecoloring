#pragma once

#include <vector>
#include <opencv2/opencv.hpp>
#include <fstream>
//using namespace cv;
using namespace std;

struct Feature {
	double r, g, b, x, y, s1, s2, s3;
	Feature(double r_ = 0, double g_  = 0, double b_  = 0, double x_  = 0,
			double y_ = 0, double s1_ = 0, double s2_ = 0, double s3_ = 0) {
		r = r_;
		g = g_;
		b = b_;
		x = x_;
		y = y_;
		s1 = s1_;
		s2 = s2_;
		s3 = s3_;
	}

	cv::Vec3d GetColor() {
		return cv::Vec3d(r, g, b);
	}

	cv::Vec2d GetCoord() {
		return cv::Vec2d(x, y);
	}

	cv::Vec3d GetSemantic() {
		return cv::Vec3d(s1, s2, s3);
	}

	Feature operator - (Feature f) {
		return Feature(r - f.r, g - f.g, b - f.b, x - f.x, y - f.y, s1 - f.s1, s2 - f.s2, s3 - f.s3);
	}

	void operator += (Feature f) {
		r += f.r;
		g += f.g;
		b += f.b;
		x += f.x;
		y += f.y;
		s1 += f.s1;
		s2 += f.s2;
		s3 += f.s3;
	}

	void operator -= (Feature f) {
		r -= f.r;
		g -= f.g;
		b -= f.b;
		x -= f.x;
		y -= f.y;
		s1 -= f.s1;
		s2 -= f.s2;
		s3 -= f.s3;
	}

	void operator /= (int n) {
		r /= n;
		g /= n;
		b /= n;
		x /= n;
		y /= n;
		s1 /= n;
		s2 /= n;
		s3 /= n;
	}

	double norm() {
		return sqrt(r* r + g * g + b * b + x * x + y * y + s1 * s1 + s2 * s2 + s3 * s3);
	}

	bool operator < (Feature f) {
		return x < f.x;
	}
};

inline double GetColorDistanceOf(Feature f1, Feature f2) {
	cv::Vec3d color_diff(f1.r - f2.r, f1.g - f2.g, f1.b - f2.b);
	double dis_color = norm(color_diff);
	return dis_color;
}

inline double GetPositionDistanceOf(Feature f1, Feature f2) {
	cv::Vec2d space_diff(f1.x - f2.x, f1.y - f2.y);
	double dis_space = norm(space_diff);
	return dis_space;
}

inline double GetSemanticDistanceOf(Feature f1, Feature f2) {
	cv::Vec3d seman_diff(f1.s1 - f2.s1, f1.s2 - f2.s2, f1.s3 - f2.s3);
	double dis_seman = norm(seman_diff);
	return dis_seman;
}

inline double GetWeightedDistanceOf(Feature f1, Feature f2) {
	double w_color = 1;
	double w_space = 0.2;
	double w_semantic = 5;

	cv::Vec3d color_diff(f1.r - f2.r, f1.g - f2.g, f1.b - f2.b);
	cv::Vec2d space_diff(f1.x - f2.x, f1.y - f2.y);
	cv::Vec3d seman_diff(f1.s1 - f2.s1, f1.s2 - f2.s2, f1.s3 - f2.s3);
	double dis_color = norm(color_diff);
	double dis_space = norm(space_diff);
	double dis_seman = norm(seman_diff);
	double dis = w_color * dis_color +  w_space * dis_space + w_semantic * dis_seman;
	return dis;
}

struct Image {
	vector<Feature> features; //n*8: rgbxy + cnn_feature
	int img_h, img_w, img_c;

	Image() {
		img_h = 0;
		img_w = 0;
		img_c = 0;
	}

	Image(string img_path, string cnn_feat_path) {
		cv::Mat img = cv::imread(img_path);
		img_h = img.rows;
		img_w = img.cols;
		img_c = img.channels();
		features.resize(img_h * img_w);

	#pragma omp parallel for
		for (int row = 0; row < img_h; row++) {
			uchar* data = img.ptr<uchar>(row);
			for (int col = 0; col < img_w; col++) {
				uchar B = data[img_c * col + 0];
				uchar G = data[img_c * col + 1];
				uchar R = data[img_c * col + 2];

				int k = row * img_w + col;
				features[k].r = R / 255.0;
				features[k].g = G / 255.0;
				features[k].b = B / 255.0;
				features[k].x = row / (img_h - 1.0);
				features[k].y = col / (img_w - 1.0);
			}
		}

		int n = img_h * img_w;
		double* feats = new double[n * 3];
		ifstream in(cnn_feat_path, ios::in | ios::binary);
		in.read((char*)feats, n * 3 * sizeof(double));

		for (int i = 0; i < n; i++) {
			features[i].s1 = feats[3 * i + 0];
			features[i].s2 = feats[3 * i + 1];
			features[i].s3 = feats[3 * i + 2];
		}
		in.close();
		free(feats);
	}

	vector<int> GetNeighborsOf(int pos) {
		int r = pos / img_w, c = pos % img_w;
		int r_[8] = { -1,0,1,0,-1,1,-1,1 };
		int c_[8] = { 0,1,0,-1,-1,1,1,-1 };
		vector<int> nb;
		for (int i = 0; i < 8; i++) {
			int new_r = r + r_[i];
			int new_c = c + c_[i];
			if (0 <= new_r && new_r < img_h && 0 <= new_c && new_c < img_w)
				nb.push_back(new_r * img_w + new_c);
		}
		return nb;
	}

	vector<Feature> GetFeaturesWith(vector<cv::Vec2i>& pos) {
		vector<Feature> feats(pos.size());
		for (int i = 0; i < pos.size(); i++) {
			int id = pos[i][0] * img_w + pos[i][1];
			feats[i] = features[id];
		}
		return feats;
	}
};