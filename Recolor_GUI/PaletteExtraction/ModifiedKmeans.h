#pragma once
#include "Utility.h"
#include <algorithm>

class ModifiedKmeans {
private:
	int m_K;
	vector<Feature> m_data;
	vector<Feature> m_centers;
	vector<Feature> m_image;
	vector<int> m_weights;
	vector<int> m_labels;

public:
	ModifiedKmeans() {
		m_K = 5;
	}

	ModifiedKmeans(vector<Feature>& feats,vector<int> weights, int k) {
		m_K = k;
		m_data = feats;
		m_weights = weights;
		m_labels.resize(m_data.size());
		m_centers.resize(m_K);

		Clustering();
	}

	vector<Feature> GetClusterCenters() {
		return m_centers;
	}

	void SelectInitCenters() {
		double sigma = 1.0;
		double sigma2 = sigma * sigma;
		vector<bool> selected(m_data.size(), false);
		
		for (int i = 0; i < m_K; i++) {
			//1. select the data with max weights as the initial center
			int max_id = 0, max_w = -1;
			for (int j = 0; j < m_weights.size(); j++) {
				if (!selected[j] && (max_w < m_weights[j])) {
					max_w = m_weights[j];
					max_id = j;
				}	
			}
			m_centers[i] = m_data[max_id];
			selected[max_id] = true;

			//2. update other data's weights
			double dis = 0, w = 0;
			for (int j = 0; j < m_data.size(); j++) {
				if (!selected[j]) {
					dis = GetWeightedDistanceOf(m_centers[i], m_data[j]);
					w = 1 - exp(-dis * dis / sigma2);
					m_weights[j] = w * m_weights[j];
				}
			}
		}
	}

	void SetInitCenters_Debug() {
		/*
		* // tv's centers
		vector<cv::Vec2d> coords(5);
		coords[0] = cv::Vec2d(185.0 / 960, 290.0 / 640);
		coords[1] = cv::Vec2d(444.0 / 960, 325.0 / 640);
		coords[2] = cv::Vec2d(469.0 / 960, 307.0 / 640);
		coords[3] = cv::Vec2d(606.0 / 960, 326.0 / 640);
		coords[4] = cv::Vec2d(925.0 / 960, 305.0 / 640);
		*/

		vector<cv::Vec2d> coords(6);
		coords[0] = cv::Vec2d(100.0 / 702,  40.0 / 463);
		coords[1] = cv::Vec2d( 80.0 / 702, 200.0 / 463);
		coords[2] = cv::Vec2d(200.0 / 702, 240.0 / 463);
		coords[3] = cv::Vec2d(250.0 / 702, 235.0 / 463);
		coords[4] = cv::Vec2d(440.0 / 702, 220.0 / 463);
		coords[5] = cv::Vec2d(560.0 / 702, 320.0 / 463);

		for (int i = 0; i < coords.size(); i++) {
			double min_id = 0, min_dis = 1e8;
			for (int j = 0; j < m_data.size(); j++) {
				double dis = norm(coords[i] - m_data[j].GetCoord());
				if (min_dis > dis) {
					min_dis = dis;
					min_id = j;
				}
			}
			m_centers[i] = m_data[min_id];
		}
	}

	void Clustering() {
		//1.select initial centers
		SelectInitCenters();

		//2.perform k-means clustering
		int it = 0;
		double diff = 0;
		while (it++ < 100) {
			UpdateDataLabel();
			vector<Feature> old_centers = m_centers;
			UpdateClusterCenter();

			diff = 0;
			for (int i = 0; i < m_centers.size(); i++)
				diff += (m_centers[i] - old_centers[i]).norm();
			if (diff < 0.0001) break;
		}

		cout << "The resulting cluster centers:" << endl;
		for (int j = 0; j < m_centers.size(); j++) {
			cout << m_centers[j].r  << "," << m_centers[j].g  << "," << m_centers[j].b  <<
				    m_centers[j].s1 << "," << m_centers[j].s2 << "," << m_centers[j].s3 << endl;
		}
		cout << endl;
	}

	void UpdateDataLabel() {
		for (int i = 0; i < m_data.size(); i++) {
			int min_center_id = 0;
			double min_dis = DBL_MAX, dis = 0;
			for (int j = 0; j < m_centers.size(); j++) {
				dis = GetWeightedDistanceOf(m_data[i], m_centers[j]);
				if (min_dis > dis) {
					min_dis = dis;
					min_center_id = j;
				}
			}
			m_labels[i] = min_center_id;
		}
	}

	void UpdateClusterCenter() {
		m_centers = vector<Feature>(m_K);
		vector<int> data_num_in_each_cluster(m_K, 0);

		for (int i = 0; i < m_data.size(); i++) {
			int label = m_labels[i];
			m_centers[label] += m_data[i];
			data_num_in_each_cluster[label]++;
		}

		for (int i = 0; i < m_K; i++)
			m_centers[i] /= data_num_in_each_cluster[i];
	}

	vector<Feature> DrawKmenasResultEx(string img_path) {
		cv::Mat rgb_img = cv::imread(img_path);

		/*
		for (int i = 0; i < m_centers.size(); i++) {
			int row = m_centers[i].x * rgb_img.rows;
			int col = m_centers[i].y * rgb_img.cols;
			cout << row << " "<<col << endl;
			circle(rgb_img, cv::Point(col, row), 5, cv::Scalar(255, 0, 0), 2);
			putText(rgb_img, to_string(i), cv::Point(col, row), FONT_HERSHEY_PLAIN, 2.0, (100, 200, 200), 1);
		}
		*/

		for (int i = 0; i < m_centers.size(); i++) {
			double min_id = 0, min_dis = 1e8;
			for (int j = 0; j < m_data.size(); j++) {
				double dis = norm(m_centers[i].GetSemantic() - m_data[j].GetSemantic());
				if (min_dis > dis) {
					min_dis = dis;
					min_id = j;
				}
			}
			int row = m_data[min_id].x * rgb_img.rows;
			int col = m_data[min_id].y * rgb_img.cols;

			circle(rgb_img, cv::Point(col, row), 5, cv::Scalar(255, 0, 0), 2);
			putText(rgb_img, to_string(i), cv::Point(col, row), cv::FONT_HERSHEY_PLAIN, 2.0, (100, 200, 200), 1);
		}
		int img_h = rgb_img.rows;
		int img_w = rgb_img.cols;
		int img_c = rgb_img.channels();
		m_image.resize(img_h * img_w);

		for (int row = 0; row < rgb_img.rows; row++) {
			uchar* data = rgb_img.ptr<uchar>(row);
			for (int col = 0; col < rgb_img.cols; col++) {
				uchar B = data[img_c * col + 0];
				uchar G = data[img_c * col + 1];
				uchar R = data[img_c * col + 2];

				int k = row * img_w + col;
				m_image[k].r = R / 255.0;
				m_image[k].g = G / 255.0;
				m_image[k].b = B / 255.0;
				m_image[k].x = row / (img_h - 1.0);
				m_image[k].y = col / (img_w - 1.0);
			}
		}

		return m_image;
		/*int pos = img_path.rfind('.');
		string kms_path = img_path.substr(0, pos) + "_kmeans.jpg";

		cv::imwrite(kms_path, rgb_img);
		cv::waitKey(0);*/
	}

	void DrawKmenasResult(string img_path) {
		cv::Mat rgb_img = cv::imread(img_path);

		/*
		for (int i = 0; i < m_centers.size(); i++) {
			int row = m_centers[i].x * rgb_img.rows;
			int col = m_centers[i].y * rgb_img.cols;
			cout << row << " "<<col << endl;
			circle(rgb_img, cv::Point(col, row), 5, cv::Scalar(255, 0, 0), 2);
			putText(rgb_img, to_string(i), cv::Point(col, row), FONT_HERSHEY_PLAIN, 2.0, (100, 200, 200), 1);
		}
		*/

		for (int i = 0; i < m_centers.size(); i++) {
			double min_id = 0, min_dis = 1e8;
			for (int j = 0; j < m_data.size(); j++) {
				double dis = norm(m_centers[i].GetSemantic() - m_data[j].GetSemantic());
				if (min_dis > dis) {
					min_dis = dis;
					min_id = j;
				}
			}
			int row = m_data[min_id].x * rgb_img.rows;
			int col = m_data[min_id].y * rgb_img.cols;

			circle(rgb_img, cv::Point(col, row), 5, cv::Scalar(255, 0, 0), 2);
			putText(rgb_img, to_string(i), cv::Point(col, row), cv::FONT_HERSHEY_PLAIN, 2.0, (100, 200, 200), 1);
		}

		int pos = img_path.rfind('.');
		string kms_path = img_path.substr(0, pos) + "_kmeans.jpg";

		cv::imwrite(kms_path, rgb_img);
		cv::waitKey(0);
	}

	void VisualizeKmenas(string img_path) {

		vector<cv::Vec3b> colors(8);
		colors[0] = cv::Vec3b(255, 0, 0);
		colors[1] = cv::Vec3b(0, 255, 0);
		colors[2] = cv::Vec3b(0, 0, 255);
		colors[3] = cv::Vec3b(255, 255, 0);
		colors[4] = cv::Vec3b(255, 0, 255);
		colors[5] = cv::Vec3b(0, 255, 255);
		colors[6] = cv::Vec3b(0, 0, 0);
		colors[7] = cv::Vec3b(255, 255, 255);

		cv::Mat rgb_img = cv::imread(img_path);
		for (int j = 0; j < m_data.size(); j++) {
			int row = m_data[j].x * rgb_img.rows;
			int col = m_data[j].y * rgb_img.cols;
			int label = m_labels[j];
			circle(rgb_img, cv::Point(col, row), 5, cv::Scalar(colors[label][0], colors[label][1], colors[label][2]), 2);
		}

		int pos = img_path.rfind('.');
		string kms_path = img_path.substr(0, pos) + "_kmeans1.jpg";

		cv::imwrite(kms_path, rgb_img);
		cv::waitKey(0);

	}
};