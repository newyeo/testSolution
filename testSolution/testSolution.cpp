// testSolution.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <stdlib.h>
#include "Solution.h"
#include "http.h"
#include <opencv2/opencv.hpp>
#include <time.h>

using namespace std;

string checkOcr(cv::Mat part)
{
	std::vector<uchar> encode_image;
	vector<int> compression_params;
	compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
	compression_params.push_back(80);
	imencode(".jpg", part, encode_image, compression_params);

	char *data = new char[encode_image.size()];
	for (int i = 0; i < encode_image.size(); ++i)
		data[i] = encode_image[i];
	// 调用通用文字识别, 图片参数为本地图片

	CWininetHttp http;
	std::map<string, string> keys;
	std::string response;

	keys.insert(pair <string, string>("type", "test1"));
	keys.insert(pair <string, string>("terminal", "test2"));
	kmvc::ERROR_CODE code = http.imageUploadRequest("http://10.33.194.49:5000/upload",
		data, encode_image.size(), "detect.jpg", keys, response);

	delete[] data;
	data = NULL;
	return response;
}

int _tmain(int argc, _TCHAR* argv[])
{
	int i = 170;
	Solution solu;
	while (i < 10e6)
	{
		clock_t start, finish;
		double totaltime;
		start = clock();

		cv::Mat img = cv::imread("D:\\test\\img4\\Image_w1280_h960_fn" + to_string(i) + ".jpg");
		//cv::Mat img = cv::imread("D:\\test\\img5\\Image_w1280_h960_fn252.jpg");
		i++;
		cout << i << endl;
		if (!img.data)
		{
			continue;
			//cout << "loading image failed;" << endl;
			//return ERROR;
		}
		/******* image process *********/
		cv::Mat imBw, imRoi, imAddRect, imMorphClose, imBwLow, imAddRectEx, imLowMorphClose;
		img.copyTo(imAddRect);
		img.copyTo(imAddRectEx);
		cv::Mat matMean, matStd;
		//solu.figShow("img", img);

		/*图像亮度调整*/
		cv::Mat imgAlpha, imgAlphaBw;
		solu.expoCorrect(img, imgAlpha);
		//solu.figShow("imgAlpha", imgAlpha);
		imgAlpha.copyTo(imAddRect);

		/*logo定位*/
		solu.norThresh(img, imBwLow,15);
		solu.norThresh(imgAlpha, imgAlphaBw, 15);
		//solu.figShow("imAlphaBw", imAlphaBw);
		//solu.figShow("imbwlow", imBwLow);
		cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(12, 4));
		cv::morphologyEx(imBwLow, imLowMorphClose, cv::MORPH_DILATE, element);
		cv::morphologyEx(imLowMorphClose, imLowMorphClose, cv::MORPH_DILATE, element);
		solu.findRectEx(imLowMorphClose, imAddRect);

		/*标签定位*/
		solu.norThresh(imgAlpha, imBw, 50);
		//solu.figShow("imBw30", imBw);
		solu.morphClose(imBw, imMorphClose, cv::Size(4, 10));
		//solu.figShow("imMorphClose", imMorphClose);
		solu.findRect(imMorphClose, imAddRect, imRoi);
		cv::Mat imLetterOcr, RoiBw, imLetterRoi;

		/*字符串定位*/
		if (imRoi.data)
		{
			imRoi.copyTo(imLetterOcr);
			solu.norThresh(imRoi, RoiBw, 90);
			cv::Mat element1 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(12, 4));
			cv::morphologyEx(RoiBw, RoiBw, cv::MORPH_ERODE, element1);
			//solu.figShow("RoiBw", RoiBw);
			cout << RoiBw.type() << endl;
			//solu.findRect(RoiBw, imLetterOcr, imLetterRoi, cv::Point(1000, 4000), cv::Point2f(3, 9));
			solu.findRect(RoiBw, imLetterOcr, imLetterRoi, cv::Point(1000, 4000), cv::Point2f(3, 9), cv::Point2f(0.5, 1));
			solu.figShow("imLetterOcr", imLetterOcr);

			/*字符串识别*/
			if (imLetterRoi.data)
			{
				//solu.figShow("letterOcr", imLetterRoi);
				//string letterOcr=checkOcr(imLetterRoi);
				//cout << "letterOcr: " << letterOcr << endl;
			}
				
		}
		cv::Mat lineonImg;
		img.copyTo(lineonImg);
		solu.findLine(imgAlphaBw, imAddRect);
		solu.figShow("imAddRect", imAddRect);

		finish = clock();
		totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
		cout << "\n此程序的运行时间为" << totaltime << "s！" << endl;
		cv::waitKey();
		
	}
	return OK;

}

