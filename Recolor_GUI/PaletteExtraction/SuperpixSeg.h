#pragma once

//https://blog.csdn.net/qq_40622955/article/details/120038943

#include <vector>
#include <opencv2/opencv.hpp>

//using namespace cv;
using namespace std;

struct spixCenter {
	int x,y; //x: column, y: row
	int L, A, B, label, pix_cnt; //pix_cnt: pixel cnt in this superpixel
};

class SuperpixSeg {
private:
	cv::Mat rgb_img, lab_img, label_mask;
	vector<spixCenter> centers; //Clustering center
	int len = 25;	//the scale of the superpixel = len*len
	int m = 10;		//adjust the weights of the spacial and color distance 

public:
	SuperpixSeg(){ }
	SuperpixSeg(string img_path, int size = 25) {  
		rgb_img = cv::imread(img_path);
		cvtColor(rgb_img, lab_img, cv::COLOR_BGR2Lab);
		len = size;
		SLIC();
	}

	vector<spixCenter> GetSuperpixCenters() {
		return centers;
	}

	//DisMask:it save the shortest distance to the nearest center
	void Clustering(cv::Mat& DisMask) {
		double cx, cy, cL, cA, cB, clabel, disc, diss, dis, *disPtr, *labelPtr;
		int L, A, B; const uchar* imgPtr;

		//note£ºx: from left to right,y:from top to bottom, (i,j) = (y,x)
		for (int ck = 0; ck < centers.size(); ++ck) {
			cx = centers[ck].x;
			cy = centers[ck].y;
			cL = centers[ck].L;
			cA = centers[ck].A;
			cB = centers[ck].B;
			clabel = centers[ck].label;

			for (int i = cy - len; i < cy + len; i++) {
				if (i < 0 || i >= lab_img.rows) continue;
				
				imgPtr = lab_img.ptr<uchar>(i); //pointer point to the ith row
				disPtr = DisMask.ptr<double>(i);
				labelPtr = label_mask.ptr<double>(i);
				for (int j = cx - len; j < cx + len; j++) {
					if (j < 0 || j >= lab_img.cols) continue;
					L = *(imgPtr + j * 3 + 0);
					A = *(imgPtr + j * 3 + 1);
					B = *(imgPtr + j * 3 + 2);

					disc = sqrt(pow(L - cL, 2) + pow(A - cA, 2) + pow(B - cB, 2));
					diss = sqrt(pow(j - cx, 2) + pow(i - cy, 2));
					dis = sqrt(pow(disc, 2) + m * pow(diss, 2));
					if (dis < *(disPtr + j)) {
						*(disPtr + j) = dis;
						*(labelPtr + j) = clabel;
					}
				}
			}
		}
	}

	int UpdateCenter() {
		double* labelPtr = NULL;
		const uchar* imgPtr = NULL;

		for (int ck = 0; ck < centers.size(); ++ck) {
			double sumx = 0, sumy = 0, sumL = 0, sumA = 0, sumB = 0, sumNum = 0;
			int cx = centers[ck].x;
			int cy = centers[ck].y;
			for (int i = cy - len; i < cy + len; i++) {
				if (i < 0 || i >= lab_img.rows) continue;
				//pointer point to the ith row
				imgPtr = lab_img.ptr<uchar>(i);
				labelPtr = label_mask.ptr<double>(i);
				for (int j = cx - len; j < cx + len; j++) {
					if (j < 0 || j >= lab_img.cols) continue;
					if (*(labelPtr + j) == centers[ck].label) {
						sumL += *(imgPtr + j * 3 + 0);
						sumA += *(imgPtr + j * 3 + 1);
						sumB += *(imgPtr + j * 3 + 2);
						sumx += j;
						sumy += i;
						sumNum += 1;
					}
				}
			}
			//update center
			if (sumNum == 0) sumNum = 0.000000001;
			centers[ck].x = sumx / sumNum;
			centers[ck].y = sumy / sumNum;
			centers[ck].L = sumL / sumNum;
			centers[ck].A = sumA / sumNum;
			centers[ck].B = sumB / sumNum;
		}
		return 0;
	}

	//replace pixel color with spixCenter's color
	void ShowRecoloredResult() {
		cv::Mat dst = rgb_img.clone();
		cvtColor(dst, dst, cv::COLOR_BGR2Lab);
		double* labelPtr = NULL;	//label_mask type: 32FC1
		uchar* imgPtr = NULL;		//image type: 8UC3
		
		for (int ck = 0; ck < centers.size(); ++ck) {
			int cx = centers[ck].x;
			int cy = centers[ck].y;
			for (int i = cy - len; i < cy + len; i++) {
				if (i < 0 || i >= rgb_img.rows) continue;
				imgPtr = dst.ptr<uchar>(i); //pointer point to the ith row
				labelPtr = label_mask.ptr<double>(i);
				for (int j = cx - len; j < cx + len; j++) {
					if (j < 0 || j >= rgb_img.cols) continue;
					if (*(labelPtr + j) == centers[ck].label){
						*(imgPtr + j * 3 + 0) = centers[ck].L;
						*(imgPtr + j * 3 + 1) = centers[ck].A;
						*(imgPtr + j * 3 + 2) = centers[ck].B;
					}
				}
			}
		}
		cv::cvtColor(dst, dst, cv::COLOR_Lab2BGR);
		cv::namedWindow("showRecolored", 0);
		cv::imshow("showRecolored", dst);
		cv::waitKey(1);
	}

	void ShowCenterAndBoundaryResult(string save_path) {
		cv::Mat dst = rgb_img.clone();
		double* labelPtr = NULL, *labelPtr_nextRow = NULL;
		uchar* imgPtr = NULL;

		for (int i = 0; i < label_mask.rows - 1; i++) {
			labelPtr = label_mask.ptr<double>(i);
			imgPtr = dst.ptr<uchar>(i);
			for (int j = 0; j < label_mask.cols - 1; j++) {
				labelPtr_nextRow = label_mask.ptr<double>(i + 1);
				//draw boundary if the left pixel's label is different from the right or bottom one
				if ((*(labelPtr+j) != *(labelPtr+j+1)) || (*(labelPtr+j) != *(labelPtr_nextRow+j))) {
					*(imgPtr + 3 * j + 0) = 0;
					*(imgPtr + 3 * j + 1) = 0;
					*(imgPtr + 3 * j + 2) = 0;
				}
			}
		}

		//draw centers
		for (int ck = 0; ck < centers.size(); ck++) {
			imgPtr = dst.ptr<uchar>(centers[ck].y);
			*(imgPtr + centers[ck].x * 3 + 0) = 100;
			*(imgPtr + centers[ck].x * 3 + 1) = 100;
			*(imgPtr + centers[ck].x * 3 + 1) = 10;
		}

		/*
		namedWindow("showBoundary", 0);
		imshow("showBoundary", dst);
		waitKey(1);
		*/

		cv::imwrite(save_path, dst);
		cv::waitKey(0);
	}

	void InitilizeCenters() {
		uchar* ptr = NULL;
		spixCenter cent;
		int num = 0;
		for (int i = 0; i < lab_img.rows; i += len) {
			cent.y = i + len / 2;
			if (cent.y >= lab_img.rows) continue;
			ptr = lab_img.ptr<uchar>(cent.y);
			for (int j = 0; j < lab_img.cols; j += len){
				cent.x = j + len / 2;
				if ((cent.x >= lab_img.cols)) continue;
				cent.L = *(ptr + cent.x * 3);
				cent.A = *(ptr + cent.x * 3 + 1);
				cent.B = *(ptr + cent.x * 3 + 2);
				cent.label = ++num;
				cent.pix_cnt = 0;
				centers.push_back(cent);
			}
		}
	}

	//if the center locates in the edges, fitune it's location.
	void FituneCenter(cv::Mat& sobelGradient) {
		if (sobelGradient.empty()) return;
		spixCenter cent;
		double* sobPtr = sobelGradient.ptr<double>(0);
		uchar* imgPtr = lab_img.ptr<uchar>(0);

		for (int ck = 0; ck < centers.size(); ck++) {
			cent = centers[ck];
			if (cent.x < 1 || cent.x + 1 >= sobelGradient.cols || cent.y < 1 || cent.y + 1 >= sobelGradient.rows)
				continue;
			double minGradient = 9999999;
			int tempx = 0, tempy = 0;
			for (int m = -1; m < 2; m++) {
				sobPtr = sobelGradient.ptr<double>(cent.y + m);
				for (int n = -1; n < 2; n++) {
					double gradient = pow(*(sobPtr + (cent.x + n) * 3), 2)
						+ pow(*(sobPtr + (cent.x + n) * 3 + 1), 2)
						+ pow(*(sobPtr + (cent.x + n) * 3 + 2), 2);
					if (gradient < minGradient) {
						minGradient = gradient;
						tempy = m;	//row
						tempx = n;	//col
					}
				}
			}
			cent.x += tempx;
			cent.y += tempy;
			imgPtr = lab_img.ptr<uchar>(cent.y);
			centers[ck].x = cent.x;
			centers[ck].y = cent.y;
			centers[ck].L = *(imgPtr + cent.x * 3);
			centers[ck].A = *(imgPtr + cent.x * 3 + 1);
			centers[ck].B = *(imgPtr + cent.x * 3 + 2);
		}
	}

	void SLIC() {
		int MAXDIS = 999999;
		int height = rgb_img.rows;
		int width = rgb_img.cols;

		cv::Mat sobelImagex, sobelImagey, sobelGradient; //get sobel gradient map
		Sobel(lab_img, sobelImagex, CV_64F, 0, 1, 3);
		Sobel(lab_img, sobelImagey, CV_64F, 1, 0, 3);
		addWeighted(sobelImagex, 0.5, sobelImagey, 0.5, 0, sobelGradient);//sobel output image type is CV_64F

		label_mask = cv::Mat::zeros(cv::Size(width, height), CV_64FC1);
		InitilizeCenters();
		FituneCenter(sobelGradient);

		for (int time = 0; time < 10; time++) {
			cv::Mat disMask(height, width, CV_64FC1, cv::Scalar(MAXDIS));
			Clustering(disMask);
			UpdateCenter();
			//FituneCenter(sobelGradient);
		}

		GetPixCntInEachSuperpix();
	}

	void GetPixCntInEachSuperpix() {
		for (int i = 0; i < label_mask.rows - 1; i++) {
			double* labelPtr = label_mask.ptr<double>(i);
			for (int j = 0; j < label_mask.cols - 1; j++) {
				int label = *(labelPtr + j);
				centers[label - 1].pix_cnt++;
			}
		}
	}

	void ShowDemo() {
		cv::namedWindow("image", 1);
		cv::imshow("image", rgb_img);
		ShowRecoloredResult();
		//ShowCenterAndBoundaryResult();
		cv::waitKey(0);
	}
};