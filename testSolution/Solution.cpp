#include "stdafx.h"
#include "Solution.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>

#define PI 3.141592653

using namespace std;

Solution::Solution()
{
}

Solution::~Solution()
{
}

void Solution::norThresh(const cv::Mat src, cv::Mat& dst, const int thresh)
{
	if (src.channels() == 3)
	{
		cv::Mat srcGray;
		cv::cvtColor(src, srcGray, cv::COLOR_BGR2GRAY);
		cv::threshold(srcGray, dst, thresh, 255, cv::THRESH_BINARY);
	}
	else
	{
		cv::threshold(src, dst, thresh, 255, cv::THRESH_BINARY);
	}
}

void Solution::adThreshhold(cv::Mat& src, cv::Mat& dst)
{
	int maxVal = 255;
	int blockSize = 41;
	double c = 0;
	if (src.channels() == 3)
		cvtColor(src, src, cv::COLOR_BGR2GRAY);
	cv::adaptiveThreshold(src, dst, maxVal, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, blockSize, c);
}

int Solution::morphClose(const cv::Mat& in, cv::Mat& out, cv::Size size)
{
	cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, size);  //调整size大小
	cv::morphologyEx(in, out, cv::MORPH_CLOSE, element);
	cv::Mat element1 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3,3));
	cv::morphologyEx(out, out, cv::MORPH_ERODE, element1);
	//MORPH_OPEN,MORPH_CLOSE,MORPH_GRADIENT,MORPH_TOPHAT,MORPH_BLACKHAT,MORPH_ERODE,MORPH_DILATE;
	//膨胀和腐蚀针对白色部分
	return OK;
}

int Solution::calNonZero(cv::Mat src, cv::Mat& dst, int thresholdValue)
{
	//计算面积
	cv::Mat threImg, blurImg;
	norThresh(src, threImg, thresholdValue);
	cv::medianBlur(threImg, blurImg,3);
	blurImg.copyTo(dst);
	return countNonZero(blurImg);
	
#if 0
	namedWindow("blurImg", WINDOW_NORMAL);
	imshow("blurImg", blurImg);
	waitKey();
#endif
}

int Solution::findLine(const cv::Mat& srcBw, cv::Mat& dst)
{
	cv::Mat imCanny;
	cv::Canny(srcBw, imCanny, 15, 230, 3);
	//figShow("imcanny", imCanny);

#if 1
	vector<cv::Vec2f> lines;
	HoughLines(imCanny, lines, 1, CV_PI / 180, 130, 0, 0);

	for (size_t i = 0; i < lines.size(); i++)
	{
		float rho = lines[i][0], theta = lines[i][1];
		//if (theta<-0.5||theta>0.5)
		//	continue;
		float rotateAngle = (float) (90.0 - theta * 180.0 / PI);
		cout << "rotateAngle: " << rotateAngle << "°"<<endl;
		if (-75 < rotateAngle&&rotateAngle < 75)
			continue;
		cout << "rho: " << rho << " theta: " << rotateAngle << endl;
		cv::Point pt1, pt2;
		double a = cos(theta), b = sin(theta);
		double x0 = a*rho, y0 = b*rho;
		pt1.x = cvRound(x0 + 1000 * (-b));
		pt1.y = cvRound(y0 + 1000 * (a));
		pt2.x = cvRound(x0 - 1000 * (-b));
		pt2.y = cvRound(y0 - 1000 * (a));
		line(dst, pt1, pt2, cv::Scalar(0, 0, 255), 2, CV_AA);
		if (0 < i&& i < lines.size()-1)
		{
			theta = abs(lines[i + 1][1] - lines[i][1]);
			if (theta >0.5)
				cout<<"this line is inclined. the rotate angle is: "<<90.0 - lines[i][1] * 180.0 / PI<<endl;
		}
		//cv::waitKey(2000);
		//figShow("linedst",dst);
	}
#else
	vector<cv::Vec4i> lines;
	HoughLinesP(imCanny, lines, 1, CV_PI / 180, 120, 50, 10);
	for (size_t i = 0; i < lines.size(); i++)
	{
		cout << "rho: " << lines[i][0] << "theta: " << lines[i][1] << endl;
		cv::Vec4i l = lines[i];
		cv::line(dst, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0, 0, 255), 3, CV_AA);
	}
#endif
	return 0;
}

//找最小包围矩形
int Solution::findRect(const cv::Mat& srcBw, cv::Mat &srcAddRect, cv::Mat& imRoi)
{
	//输入为 二值化原图，原图，保存ROI的Mat
	//找到轮廓的最小包围边框，并画出包围矩形
	cv::Mat src_copy; cv::Mat temp1;

	if (srcBw.channels() == 3)
	{
		cout << "the format of input source image is not binary" << endl;
		return -1;
	}

	//if input dstination image srcAddRect is NULL, then copy source image to it
	if (!srcAddRect.data)
		srcBw.copyTo(srcAddRect);

	srcAddRect.copyTo(src_copy);
	vector<vector<cv::Point>> contours;
	vector<cv::Vec4i> hierarchy;
	findContours(srcBw, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

	vector<cv::Rect> boundRect(contours.size());  //定义一个矩形类集合
	vector<cv::RotatedRect> rotateRect(contours.size());
	cv::Point2f pointRect[4];

	for (int i = 0; i < contours.size(); i++)
	{
		//drawContours(imageROI, contours, i, Scalar(0,0,255), 1, 8); //画出轮廓
		boundRect[i] = cv::boundingRect(cv::Mat(contours[i]));
		rotateRect[i] = cv::minAreaRect(cv::Mat(contours[i]));
		//删除过小的矩形
		//if (boundRect[i].width >(srcBw.cols - 100) || boundRect[i].height < 10 || boundRect[i].area() < 20)
		if (boundRect[i].width >(srcBw.cols - 10) || boundRect[i].height < 0.5*srcBw.rows ||
			boundRect[i].width < 0.2*srcBw.cols || boundRect[i].height > srcBw.rows-100 || boundRect[i].area() < 10000)
			continue;
		//cout << " width: " << boundRect[i].width << 0.2*srcBw.cols << " height: " << boundRect[i].height << 0.5*srcBw.rows << " area: " << boundRect[i].area() << endl;
		//删除宽高比过小的矩形
		double proportion = (double)boundRect[i].width / boundRect[i].height;
		cout << proportion << endl;
		if (proportion > 1 || proportion < 0.6)
			continue;

		//删除包围盒靠近下边界的矩形
		if (boundRect[i].tl().y + boundRect[i].height>srcBw.rows - 50)
			continue;

		//截取ROI区域，并返回ROI图片
		imRoi = src_copy(cv::Rect(boundRect[i].tl().x, boundRect[i].tl().y,
			                      boundRect[i].width, boundRect[i].height));

		rotateRect[i].points(pointRect);  //将最小外接矩形四个端点复制给pointRect数组；

		for (int j = 0; j < 4; j++)
		{
			line(srcAddRect, pointRect[j], pointRect[(j+1) % 4], cv::Scalar(0, 255, 0), 2, 8);
		}

		//画出矩形包围框，rect.tl(),为矩形类左上点坐标，rect.br()为左下点的坐标
		//drawContours(dst, contours, i, Scalar(0, 0, 255), -1, 8); //画出轮廓
		//rectangle(srcAddRect, boundRect[i].tl(), boundRect[i].br(), cv::Scalar(0, 0, 255), 2, 8, 0);
		//temp1.copyTo(dst);

		//旋转校正
		cout << rotateRect[i].angle << endl;
		float angle=rotateRect[i].angle;
		float angle0 = 0;
		if (0 < abs(angle) && abs(angle) <= 45)
			angle0 = angle;
		else if (45 < abs(angle) && abs(angle) < 90)
			angle0 = 90 - abs(angle);

		cv::Point2f center = rotateRect[i].center;
		double scale = 1;
		cv::Mat rotateMat = getRotationMatrix2D(center, angle0, scale);
		warpAffine(src_copy, src_copy, rotateMat, src_copy.size());
		imRoi = src_copy(cv::Rect(boundRect[i].tl().x, boundRect[i].tl().y,
			boundRect[i].width, boundRect[i].height));
	}
	return 0;

#if 0
	namedWindow("rect_display", WINDOW_NORMAL);
	imshow("rect_display", dst);
#endif
}

//找最小包围矩形
int Solution::findRectEx(cv::Mat srcBw, cv::Mat &srcAddRect)
{
	//输入为 二值化原图，原图，保存ROI的Mat
	//找到轮廓的最小包围边框，并画出包围矩形
	if (srcBw.channels() == 3)
	{
		cout << "the format of input source image is not binary" << endl;
		return -1;
	}

	//if input dstination image srcAddRect is NULL, then copy source image to it
	if (!srcAddRect.data)
		srcBw.copyTo(srcAddRect);
	cv::Mat src_copy; cv::Mat temp1;
	srcAddRect.copyTo(src_copy);
	vector<vector<cv::Point>> contours;
	vector<cv::Vec4i> hierarchy;
	findContours(srcBw, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

	vector<cv::Rect> boundRect(contours.size());  //定义一个矩形类集合
	vector<cv::RotatedRect> rotateRect(contours.size());
	cv::Point2f pointRect[4];

	for (int i = 0; i < contours.size(); i++)
	{
		//drawContours(imageROI, contours, i, Scalar(0,0,255), 1, 8); //画出轮廓
		boundRect[i] = cv::boundingRect(cv::Mat(contours[i]));
		rotateRect[i] = cv::minAreaRect(cv::Mat(contours[i]));
		//删除过小的矩形
		//if (boundRect[i].width >(srcBw.cols - 100) || boundRect[i].height < 10 || boundRect[i].area() < 20)
		if (boundRect[i].width >(srcBw.cols - 10) || boundRect[i].height > 0.5*srcBw.rows ||
			boundRect[i].width > 0.5*srcBw.cols || boundRect[i].height > srcBw.rows - 100 || 
			boundRect[i].area() < 6000)
			continue;
		//cout << " width: " << boundRect[i].width << 0.2*srcBw.cols << " height: " << boundRect[i].height << 0.5*srcBw.rows << " area: " << boundRect[i].area() << endl;
		//删除宽高比过小的矩形
		double proportion = (double)boundRect[i].width / boundRect[i].height;
		cout << proportion << endl;
		if (proportion < 1)
			continue;

		cout << "area: "<<boundRect[i].area() << endl;
		//删除包围盒靠近下边界的矩形
		if (boundRect[i].tl().y + boundRect[i].height>srcBw.rows - 50)
			continue;

		rotateRect[i].points(pointRect);  //将最小外接矩形四个端点复制给pointRect数组；

		for (int j = 0; j < 4; j++)
		{
			line(srcAddRect, pointRect[j], pointRect[(j + 1) % 4], cv::Scalar(255, 255, 0), 2, 8);
		}

		//画出矩形包围框，rect.tl(),为矩形类左上点坐标，rect.br()为左下点的坐标
		//drawContours(dst, contours, i, Scalar(0, 0, 255), -1, 8); //画出轮廓
		//rectangle(srcAddRect, boundRect[i].tl(), boundRect[i].br(), cv::Scalar(0, 0, 255), 2, 8, 0);
		//temp1.copyTo(dst);

		//旋转校正
		//cout << rotateRect[i].angle << endl;
		//float angle = rotateRect[i].angle;
		//float angle0;
		//if (0 < abs(angle) && abs(angle) <= 45)
		//	angle0 = angle;
		//else if (45 < abs(angle) && abs(angle) < 90)
		//	angle0 = 90 - abs(angle);
		//cv::Point2f center = rotateRect[i].center;
		//double scale = 1;
		//cv::Mat rotateMat = getRotationMatrix2D(center, angle0, scale);
		//warpAffine(src_copy, src_copy, rotateMat, src_copy.size());
		//imRoi = src_copy(cv::Rect(boundRect[i].tl().x, boundRect[i].tl().y,
		//	boundRect[i].width, boundRect[i].height));
	}
	return 0;
}

int Solution::findRect(const cv::Mat& srcBw,
					         cv::Mat& srcAddRect,
					         cv::Mat& RetRoi,
		               const cv::Point areaRange,
				       const cv::Point2f wtohRange)
{
	//find mimimum bounding rectangle and draw it on srcAddRect
	cv::Mat src_copy; cv::Mat temp1;
	srcAddRect.copyTo(src_copy);

	if (srcBw.channels()==3)
	{
		cout << "input source image is NULL or the format is not binary" << endl;
		return -1;
	}

	//find contours of input image
	vector<vector<cv::Point>> contours;
	vector<cv::Vec4i> hierarchy;
	findContours(srcBw, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

	vector<cv::Rect> boundRect(contours.size());  //定义一个矩形类集合
	vector<cv::RotatedRect> rotateRect(contours.size());
	cv::Point2f pointRect[4];

	for (int i = 0; i < contours.size(); i++)
	{
		//drawContours(imageROI, contours, i, Scalar(0,0,255), 1, 8); //画出轮廓
		boundRect[i] = cv::boundingRect(cv::Mat(contours[i]));
		rotateRect[i] = cv::minAreaRect(cv::Mat(contours[i]));
		//删除过小的矩形

		//cout << "area of boundRect: " << boundRect[i].area() << endl;
		//if (boundRect[i].width >(srcBw.cols - 100) || boundRect[i].height < 10 || boundRect[i].area() < 20)
		if (boundRect[i].width >(srcBw.cols - 10) || boundRect[i].height > 0.9*srcBw.rows ||
			boundRect[i].width > 0.9*srcBw.cols || boundRect[i].height > srcBw.rows - 100 ||
			boundRect[i].area() < areaRange.x || boundRect[i].area() > areaRange.y)
			continue;
		//cout << " width: " << boundRect[i].width << 0.2*srcBw.cols << " height: " << boundRect[i].height << 0.5*srcBw.rows << " area: " << boundRect[i].area() << endl;
		//删除宽高比过小的矩形
		double proportion = (double)boundRect[i].width / boundRect[i].height;
		cout <<"proportion is: "<< proportion << endl;
		if (wtohRange.x > proportion || proportion > wtohRange.y)
			continue;

		//cout << "area is : " << boundRect[i].area() << endl;
		//删除包围盒靠近下边界的矩形
		cout << "width*0.5: " << srcBw.cols*0.5 << endl;
		if (boundRect[i].tl().x < srcBw.cols*0.5)
			continue;

		rotateRect[i].points(pointRect);  //将最小外接矩形四个端点复制给pointRect数组；

		for (int j = 0; j < 4; j++)
		{
			line(srcAddRect, pointRect[j], pointRect[(j + 1) % 4], cv::Scalar(255, 255, 0), 2, 8);
		}

		RetRoi = src_copy(cv::Rect(boundRect[i].tl().x, boundRect[i].tl().y,
			boundRect[i].width, boundRect[i].height));

		//画出矩形包围框，rect.tl(),为矩形类左上点坐标，rect.br()为左下点的坐标
		//drawContours(dst, contours, i, Scalar(0, 0, 255), -1, 8); //画出轮廓
		//rectangle(srcAddRect, boundRect[i].tl(), boundRect[i].br(), cv::Scalar(0, 0, 255), 2, 8, 0);
		//temp1.copyTo(dst);

		//旋转校正
		//cout << rotateRect[i].angle << endl;
		//float angle = rotateRect[i].angle;
		//float angle0;
		//if (0 < abs(angle) && abs(angle) <= 45)
		//	angle0 = angle;
		//else if (45 < abs(angle) && abs(angle) < 90)
		//	angle0 = 90 - abs(angle);
		//cv::Point2f center = rotateRect[i].center;
		//double scale = 1;
		//cv::Mat rotateMat = getRotationMatrix2D(center, angle0, scale);
		//warpAffine(src_copy, src_copy, rotateMat, src_copy.size());
		//imRoi = src_copy(cv::Rect(boundRect[i].tl().x, boundRect[i].tl().y,
		//	boundRect[i].width, boundRect[i].height));
	}
	return 0;
}

int Solution::findRect(const cv::Mat& srcBw,
							 cv::Mat& srcAddRect,
							 cv::Mat& RetRoi,
					   const cv::Point areaRange,
					   const cv::Point2f wtohRange,
					   const cv::Point2f pointPositionX,
					   const cv::Point2f pointPositionY,
					   const int pointNum)
{
	//find mimimum bounding rectangle and draw it on srcAddRect
	cv::Mat src_copy; cv::Mat temp1;
	srcAddRect.copyTo(src_copy);

	if (srcBw.channels() == 3)
	{
		cout << "input source image is NULL or the format is not binary" << endl;
		return -1;
	}

	//find contours of input image
	vector<vector<cv::Point>> contours;
	vector<cv::Vec4i> hierarchy;
	findContours(srcBw, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

	vector<cv::Rect> boundRect(contours.size());  //定义一个矩形类集合
	vector<cv::RotatedRect> rotateRect(contours.size());
	cv::Point2f pointRect[4];

	for (int i = 0; i < contours.size(); i++)
	{
		//drawContours(imageROI, contours, i, Scalar(0,0,255), 1, 8); //画出轮廓
		boundRect[i] = cv::boundingRect(cv::Mat(contours[i]));
		rotateRect[i] = cv::minAreaRect(cv::Mat(contours[i]));
		//删除过小的矩形

		//cout << "area of boundRect: " << boundRect[i].area() << endl;
		//if (boundRect[i].width >(srcBw.cols - 100) || boundRect[i].height < 10 || boundRect[i].area() < 20)
		if (boundRect[i].width >(srcBw.cols - 10) || boundRect[i].height > 0.95*srcBw.rows ||
			boundRect[i].width > 0.95*srcBw.cols || boundRect[i].height > srcBw.rows - 100 ||
			boundRect[i].area() < areaRange.x || boundRect[i].area() > areaRange.y)
			continue;
		//cout << " width: " << boundRect[i].width << 0.2*srcBw.cols << " height: " << boundRect[i].height << 0.5*srcBw.rows << " area: " << boundRect[i].area() << endl;
		//删除宽高比过小的矩形
		double proportion = (double)boundRect[i].width / boundRect[i].height;
		cout << "proportion is: " << proportion << endl;
		if (wtohRange.x > proportion || proportion > wtohRange.y)
			continue;

		//cout << "area is : " << boundRect[i].area() << endl;
		//删除包围盒靠近下边界的矩形
		cv::Point boundRectPoint;
		switch (pointNum)
		{
		case 1:
			boundRectPoint = boundRect[i].tl();
			break;
		case 2:
			boundRectPoint = cv::Point(boundRect[i].tl().x, boundRect[i].tl().y + srcBw.rows);
			break;
		case 3:
			boundRectPoint = boundRect[i].br();
			break;
		case 4:
			boundRectPoint = cv::Point(boundRect[i].tl().x + srcBw.cols, boundRect[i].tl().y);
			break;
		default:
			boundRectPoint = boundRect[i].tl();
		}

		cout << "width*0.5: " << srcBw.cols*pointPositionX.x << endl;
		if (boundRectPoint.x < srcBw.cols*pointPositionX.x || boundRectPoint.x>srcBw.cols*pointPositionX.y)
			continue;
		if (boundRectPoint.y < srcBw.rows*pointPositionY.x || boundRectPoint.y>srcBw.rows*pointPositionY.y)
			continue;

		rotateRect[i].points(pointRect);  //将最小外接矩形四个端点复制给pointRect数组；

		for (int j = 0; j < 4; j++)
		{
			line(srcAddRect, pointRect[j], pointRect[(j + 1) % 4], cv::Scalar(255, 255, 0), 2, 8);
		}

		RetRoi = src_copy(cv::Rect(boundRect[i].tl().x, boundRect[i].tl().y,
			boundRect[i].width, boundRect[i].height));

		//画出矩形包围框，rect.tl(),为矩形类左上点坐标，rect.br()为左下点的坐标
		//drawContours(dst, contours, i, Scalar(0, 0, 255), -1, 8); //画出轮廓
		//rectangle(srcAddRect, boundRect[i].tl(), boundRect[i].br(), cv::Scalar(0, 0, 255), 2, 8, 0);
		//temp1.copyTo(dst);

		//旋转校正
		//cout << rotateRect[i].angle << endl;
		//float angle = rotateRect[i].angle;
		//float angle0;
		//if (0 < abs(angle) && abs(angle) <= 45)
		//	angle0 = angle;
		//else if (45 < abs(angle) && abs(angle) < 90)
		//	angle0 = 90 - abs(angle);
		//cv::Point2f center = rotateRect[i].center;
		//double scale = 1;
		//cv::Mat rotateMat = getRotationMatrix2D(center, angle0, scale);
		//warpAffine(src_copy, src_copy, rotateMat, src_copy.size());
		//imRoi = src_copy(cv::Rect(boundRect[i].tl().x, boundRect[i].tl().y,
		//	boundRect[i].width, boundRect[i].height));
	}
	return 0;
}

//亮度调整
void Solution::expoCorrect(const cv::Mat& _srcImg, cv::Mat& _expoCorrect, int _meanExpo)
{
	cv::Mat _matMean, _matStd;
	cv::meanStdDev(_srcImg, _matMean, _matStd);
	double meanVal, stdVal;
	meanVal = _matMean.at<double>(0, 0);
	stdVal = _matStd.at<double>(0, 0);
	//cout << "mean value: " << meanVal << endl;
	//cout << "std value: " << stdVal << endl;
	double alpha = _meanExpo / meanVal;
	_srcImg.convertTo(_expoCorrect, -1, alpha);
}

void Solution::figShow(std::string winName, const cv::Mat figure)
{
	cv::namedWindow(winName, CV_WINDOW_NORMAL);
	cv::imshow(winName, figure);
	cv::waitKey(20);
}

/***** 求两点间距离*****/
float Solution::getDistP2P(cv::Point pointO, cv::Point pointA)
{
	float distance;
	distance = powf((pointO.x - pointA.x), 2) + powf((pointO.y - pointA.y), 2);
	distance = sqrtf(distance);
	return distance;
}

/***** 点到直线的距离:P到AB的距离*****/
//P为线外一点，AB为线段两个端点
float Solution::getDistP2L(cv::Point pointP, cv::Point pointA, cv::Point pointB)
{
	//求直线方程
	int A = 0, B = 0, C = 0;
	A = pointA.y - pointB.y;
	B = pointB.x - pointA.x;
	C = pointA.x*pointB.y - pointA.y*pointB.x;
	//代入点到直线距离公式
	float distance = 0;
	distance = ((float)abs(A*pointP.x + B*pointP.y + C)) / ((float)sqrtf(A*A + B*B));
	return distance;
}
