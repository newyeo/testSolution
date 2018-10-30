#pragma once
#include <opencv2/opencv.hpp>

class Solution
{
public:
	Solution();
	~Solution();
	int   calNonZero(cv::Mat src, cv::Mat& dst, int thresholdValue = 30);
	void  adThreshhold(cv::Mat& src,cv::Mat& dst);
	void  norThresh(const cv::Mat src, cv::Mat& dst, const int thresh = 200);
	void  figShow(const std::string winName, const cv::Mat figure);
	int   morphClose(const cv::Mat& in, cv::Mat& out, cv::Size size);
	int   findLine(const cv::Mat& srcBw, cv::Mat& dst);
	float getDistP2L(cv::Point pointP, cv::Point pointA, cv::Point pointB);
	float getDistP2P(cv::Point pointO, cv::Point pointA);

	//find rectange of region of intest of input image
	int   findRect(const cv::Mat& srcBw, cv::Mat &srcAddRect, cv::Mat& imRoi);

	//find Rect
	int   findRectEx(cv::Mat srcBw, cv::Mat &srcAddRect);

	//override findRect
	int   findRect(const cv::Mat& srcBw, cv::Mat& srcAddRect, cv::Mat& RetRoi,
		           const cv::Point areaRange, const cv::Point2f wtohRange);

	int findRect(const cv::Mat& srcBw, cv::Mat& srcAddRect, cv::Mat& RetRoi,
				 const cv::Point areaRange, const cv::Point2f wtohRange,
				 const cv::Point2f pointPositionX, const cv::Point2f pointPositionY = cv::Point2f(0,1), 
				 const int pointNum = 1);

	//correct exposure time
	void expoCorrect(const cv::Mat& srcImg, cv::Mat& _expoCorrect, int _meanExpo = 31);


};

