#ifndef UTILS_H
#define UTILS_H

#include<string>
#include<vector>
#include<Windows.h>
#include<opencv2/opencv.hpp>


struct QcObject{
    int Class;
    std::string Name;
    cv::Scalar Color;
};

QcObject getQcObject(double objclass);

char* bufcat(char*dest,const char*src,int len);

std::string getImgName(std::string path);

char * readImgFile(const char *pathpic, int &pic_len);

void parseURLWeb( std::string strUrl, std::string& strHostName,
                  std::string& strPageName, WORD& sPort);

void parseDetectResponse(const std::string &response,
                         std::vector<std::vector<double>>&response_parse);

//void dispDetectObject(cv::Mat& img, const cv::Rect &rec,
//                      const std::string& text, cv::Scalar &sca, int thick);

void drawDetectObject(cv::Mat &img, cv::Point &p1, cv::Point &p2,
                      const std::string &text, cv::Scalar &sca,double fontscalse,double thickness);

void drawDetectObjects(cv::Mat &img,std::vector<std::vector<double>>& objects,double scaleW,
                       double scaleH,double fontscale,double thickness);

void getOptimizePath(cv::Point2d &beginPt,std::vector<cv::Point2d>&srcPts,
                     std::vector<int>&order,std::string method = "greed");

double evaluateBlur(cv::Mat &image,std::string method);

void GetStringSize(HDC hDC, const char* str, int* w, int* h);

void putTextZH(cv::Mat &dst, const char* str, cv::Point org, cv::Scalar color, int fontSize,
    const char *fn = "Arial", bool italic = false, bool underline = false);
#endif // UTILS_H
