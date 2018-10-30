#include "stdafx.h"
#include "utils.h"
#include<sstream>
//#include<QtQDomDocument>
#include <QtXml/QDom.h>
using namespace std;
using namespace cv;

char* bufcat(char*dest,const char*src,int len)
{
    while(len-->0)
        *dest++ = *src++;
    return dest;
}

string getImgName(string path)
{
    std::size_t found = path.find_last_of("/\\");
    string re = "";
    if(found > 0)
        re = path.substr(found+1);
    return re;
}

char * readImgFile(const char *pathpic, int &pic_len)
{
    FILE *fp = fopen(pathpic, "rb");
    if (!fp){
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    pic_len = ftell(fp);
    rewind(fp);
    char *pic_buf = new char[pic_len + 1];
    memset(pic_buf, 0, pic_len + 1);
    fread(pic_buf,sizeof(char),pic_len,fp);
    fclose(fp);
    return pic_buf;
}

// 解析URL地址 [3/14/2017/shike]
void parseURLWeb( std::string strUrl, std::string& strHostName, std::string& strPageName, WORD& sPort)
{
    sPort = 80;
    string strTemp(strUrl);
    std::size_t nPos = strTemp.find("http://");
    if (nPos != std::string::npos){
        strTemp = strTemp.substr(nPos + 7, strTemp.size() - nPos - 7);
    }

    nPos = strTemp.find('/');
    if ( nPos == std::string::npos )    //没有找到
    {
        strHostName = strTemp;
    }
    else
    {
        strHostName = strTemp.substr(0, nPos);
    }

    std::size_t nPos1 = strHostName.find(':');
    if ( nPos1 != std::string::npos )
    {
        std::string strPort = strTemp.substr(nPos1 + 1, strHostName.size() - nPos1 - 1);
        strHostName = strHostName.substr(0, nPos1);
        sPort = (WORD)atoi(strPort.c_str());
    }
    if ( nPos == std::string::npos )
    {
        return ;
    }
    strPageName = strTemp.substr(nPos, strTemp.size() - nPos);
}

void parseDetectResponse(const string &response, std::vector<std::vector<double> > &response_parse)
{
    size_t re = response.find_last_of('#');
    if(re == string::npos)
        return;
    string result = response.substr(re+1);
    size_t endpo = result.find_last_of('$');
    if(endpo == string::npos)
        return;
    size_t pos = 0;
    const char* data = result.c_str();
    pos = result.find_first_of(':',pos+1);
    while(pos!= string::npos && pos < endpo)
    {
        vector<double>numbers;
        int count = 0;
        int i = 0;
        for(i=pos+1;data[i]!='o'&&data[i]!='$';++i)
        {
           if(data[i] != ',')
               count++;
           else
           {
               string sub = result.substr(i-count,count);
               istringstream iss(sub);
               double num;
               iss>>num;
               numbers.push_back(num);
               count = 0;
           }
        }
        if(numbers[1]>0.95)
            response_parse.push_back(numbers);
        if(data[i] == '$')
            break;
        pos = result.find_first_of(":",pos+1);
    }
}

QcObject getQcObject(double objclass)
{
    QcObject obj;
    int cls = int(std::round(objclass));
    obj.Class = cls;
    switch (cls) {
    case 1:
        obj.Color = cv::Scalar(0, 255, 0);
        obj.Name = "Bolt";
        break;
    case 2:
        obj.Color = cv::Scalar(255,0,0);
        obj.Name = "Wrnus_lab";
        break;
    case 3:
        obj.Color = cv::Scalar(0,0,255);
        obj.Name = "Wrnds_lab";
        break;
    case 4:
        obj.Color = cv::Scalar(100, 149, 237);
        obj.Name = "Prod_lab";
        break;
    case 5:
        obj.Color = cv::Scalar(0, 100, 0);
        obj.Name = "Ener_lab";
        break;
    case 6:
        obj.Color = cv::Scalar(205, 155, 29);
        obj.Name = "Cert_lab";
        break;
    case 7:
        obj.Color = cv::Scalar(139, 71, 38);
        obj.Name = "QR_Code";
        break;
    case 8:
        obj.Color = cv::Scalar(199, 21, 133);
        obj.Name = "Wrnbk_lab";
        break;
    case 9:
        obj.Color = cv::Scalar(200, 21, 203);
        obj.Name = "orc_lab";
        break;
    case 10:
        obj.Color = cv::Scalar(130, 21, 103);
        obj.Name = "logo_lab";
        break;
    default:
        break;
    }
    return obj;
}

void drawDetectObject(Mat &img, Point &p1, Point &p2,
                      const string &text, Scalar &sca,double fontScale,double thickness)
{
    //double font_scale = 2.5;

    cv::rectangle(img,p1,p2,sca,2);
    int font_face = cv::FONT_HERSHEY_COMPLEX;

    int baseline;
    cv::Size text_size = cv::getTextSize(text, font_face, fontScale, thickness, &baseline);
    cv::Point origin;
    origin.x = p1.x;
    if(p1.y - text_size.height/2 > 1)
        origin.y = p1.y - text_size.height/2;
    else
        origin.y = p2.y + 1.5*text_size.height;
    cv::putText(img, text, origin, font_face, fontScale, sca, thickness);
    putTextZH(img,text.c_str(),origin,sca,30, "Arial");
}


string dToStr(double num)
{
    char str[256];
    sprintf(str, "%lf", num);
   // sprintf(str, "%0.*f", 2, num);
    string result = str;
    return result.substr(0,5);
}
void drawDetectObjects(Mat &img, std::vector<vector<double> > &objects, double scaleW,double scaleH,double fontscale,double thickness)
{
    vector<vector<double>>::iterator iter;
       // cv::resize(img,img,cv::Size(img.cols*scale,img.rows*scale));
    cv::resize(img,img,Size(),scaleW,scaleH);

    for(iter = objects.begin();iter!=objects.end();++iter)
    {
        cv::Scalar color;
        std::string text;
        QcObject obj;

        obj = getQcObject((*iter)[0]);
        text += obj.Name;
        text += " ";
        text += dToStr((*iter)[1]);
        color = obj.Color;
        drawDetectObject(img,Point((*iter)[2]*scaleW,(*iter)[3]*scaleH),Point((*iter)[4]*scaleW,(*iter)[5]*scaleH),
                text,color,fontscale,thickness);
    }
}

void swap(vector<int> &lst, int i, int j)
{
   int tmp = lst[i];
   lst[i] = lst[j];
   lst[j] = tmp;
}

void perm(vector<int> &lst, int start, int end, vector<vector<int> > &dst)
{
    if (start >= end) {
        dst.push_back(lst);
    } else {
        for (int i=start;i<end;i++) {
            swap(lst, start, i);
            perm(lst, start+1, end, dst);
            swap(lst, start, i);
        }
    }
}

double distancePP(Point2d &p1, Point2d &p2)
{
    return std::sqrt((p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y));
}

void getPath_Best(Point2d beginPt,vector<Point2d>&srcPts,vector<int>&order)
{
    vector<double>dists;
    vector<int>num;
    vector<vector<int>> combs;

    int size = srcPts.size();
    for(int i = 0;i < size;++i){
        num.push_back(i);
        for(int j=0;j<size;++j){
            dists.push_back(distancePP(srcPts[i],srcPts[j]));
        }
    }
    perm(num,0,size,combs);
    double minDist = numeric_limits<double>::max();
    vector<int>track;
    for(int i=0;i<combs.size();++i){
        double d = 0;
        vector<int>p = combs[i];
        d += distancePP(beginPt,srcPts[p[0]]);
        for(int j=0;j<p.size()-1;j++){
            d+=dists[p[j]*size+p[j+1]];
        }
        d+=distancePP(srcPts[p[size-1]],beginPt);
        if(d < minDist){
            minDist = d;
            track = p;
        }
    }
    order = track;
}

void getPath_Greed(Point2d& beginPt,vector<Point2d>&srcPts,vector<int>&order)
{
    bool lable[100];
    int number = 0;

    number = srcPts.size();
    for(int i=0;i<100;i++)
        lable[i]=false;

    Point2d curr = beginPt;
    for(int i=0;i<number;++i){
        double mindist = 999999.99999;
        int po = -1;
        for(int j=0;j<number;++j){
            if(lable[j]==true)
                continue;
            double dis = distancePP(curr,srcPts[j]);
            if(dis<mindist){
                mindist = dis;
                po = j;
            }
        }
        if(po >= 0){
            lable[po] = true;
            curr = srcPts[po];
            order.push_back(po);
        }
    }
}

void getOptimizePath(Point2d& beginPoint,vector<Point2d>&srcPts,vector<int>&order,string method)
{
    if(method == "best")
        getPath_Best(beginPoint,srcPts,order);
    else if(method=="greed")
        getPath_Greed(beginPoint,srcPts,order);
}

double evaluateBlur(Mat &image, string method)
{
    Mat imageSobel;
    image.copyTo(imageSobel);
    Sobel(imageSobel, imageSobel, CV_16U, 1, 1);

    //图像的平均灰度
    double meanValue = 0.0;
    meanValue = mean(imageSobel)[0];
    return meanValue;
}

void GetStringSize(HDC hDC, const char* str, int* w, int* h)
{
//    SIZE size;
//    GetTextExtentPoint32A(hDC, str, strlen(str), &size);
//    if (w != 0) *w = size.cx;
//    if (h != 0) *h = size.cy;
}

void putTextZH(Mat &dst, const char* str, Point org, Scalar color, int fontSize, const char* fn, bool italic, bool underline)
{
//    CV_Assert(dst.data != 0 && (dst.channels() == 1 || dst.channels() == 3));
//    int x, y, r, b;
//    if (org.x > dst.cols || org.y > dst.rows) return;
//    x = org.x < 0 ? -org.x : 0;
//    y = org.y < 0 ? -org.y : 0;

//    LOGFONTA lf;
//    lf.lfHeight = -fontSize;
//    lf.lfWidth = 0;
//    lf.lfEscapement = 0;
//    lf.lfOrientation = 0;
//    lf.lfWeight = 5;
//    lf.lfItalic = italic;   //斜体
//    lf.lfUnderline = underline; //下划线
//    lf.lfStrikeOut = 0;
//    lf.lfCharSet = DEFAULT_CHARSET;
//    lf.lfOutPrecision = 0;
//    lf.lfClipPrecision = 0;
//    lf.lfQuality = PROOF_QUALITY;
//    lf.lfPitchAndFamily = 0;
//    strcpy_s(lf.lfFaceName, fn);

//    HFONT hf = CreateFontIndirectA(&lf);
//    HDC hDC = CreateCompatibleDC(0);
//    HFONT hOldFont = (HFONT)SelectObject(hDC, hf);

//    int strBaseW = 0, strBaseH = 0;
//    int singleRow = 0;
//    char buf[1 << 12];
//    strcpy_s(buf, str);
//    char *bufT[1 << 12];  // 这个用于分隔字符串后剩余的字符，可能会超出。
//    //处理多行
//    {
//        int nnh = 0;
//        int cw, ch;

//        const char* ln = strtok_s(buf, "\n",bufT);
//        while (ln != 0)
//        {
//            GetStringSize(hDC, ln, &cw, &ch);
//            strBaseW = max(strBaseW, cw);
//            strBaseH = max(strBaseH, ch);

//            ln = strtok_s(0, "\n",bufT);
//            nnh++;
//        }
//        singleRow = strBaseH;
//        strBaseH *= nnh;
//    }

//    if (org.x + strBaseW < 0 || org.y + strBaseH < 0)
//    {
//        SelectObject(hDC, hOldFont);
//        DeleteObject(hf);
//        DeleteObject(hDC);
//        return;
//    }

//    r = org.x + strBaseW > dst.cols ? dst.cols - org.x - 1 : strBaseW - 1;
//    b = org.y + strBaseH > dst.rows ? dst.rows - org.y - 1 : strBaseH - 1;
//    org.x = org.x < 0 ? 0 : org.x;
//    org.y = org.y < 0 ? 0 : org.y;

//    BITMAPINFO bmp = { 0 };
//    BITMAPINFOHEADER& bih = bmp.bmiHeader;
//    int strDrawLineStep = strBaseW * 3 % 4 == 0 ? strBaseW * 3 : (strBaseW * 3 + 4 - ((strBaseW * 3) % 4));

//    bih.biSize = sizeof(BITMAPINFOHEADER);
//    bih.biWidth = strBaseW;
//    bih.biHeight = strBaseH;
//    bih.biPlanes = 1;
//    bih.biBitCount = 24;
//    bih.biCompression = BI_RGB;
//    bih.biSizeImage = strBaseH * strDrawLineStep;
//    bih.biClrUsed = 0;
//    bih.biClrImportant = 0;

//    void* pDibData = 0;
//    HBITMAP hBmp = CreateDIBSection(hDC, &bmp, DIB_RGB_COLORS, &pDibData, 0, 0);

//    CV_Assert(pDibData != 0);
//    HBITMAP hOldBmp = (HBITMAP)SelectObject(hDC, hBmp);

//    //color.val[2], color.val[1], color.val[0]
//    SetTextColor(hDC, RGB(255, 255, 255));
//    SetBkColor(hDC, 0);
//    //SetStretchBltMode(hDC, COLORONCOLOR);

//    strcpy_s(buf, str);
//    const char* ln = strtok_s(buf, "\n",bufT);
//    int outTextY = 0;
//    while (ln != 0)
//    {
//        TextOutA(hDC, 0, outTextY, ln, strlen(ln));
//        outTextY += singleRow;
//        ln = strtok_s(0, "\n",bufT);
//    }
//    uchar* dstData = (uchar*)dst.data;
//    int dstStep = dst.step / sizeof(dstData[0]);
//    unsigned char* pImg = (unsigned char*)dst.data + org.x * dst.channels() + org.y * dstStep;
//    unsigned char* pStr = (unsigned char*)pDibData + x * 3;
//    for (int tty = y; tty <= b; ++tty)
//    {
//        unsigned char* subImg = pImg + (tty - y) * dstStep;
//        unsigned char* subStr = pStr + (strBaseH - tty - 1) * strDrawLineStep;
//        for (int ttx = x; ttx <= r; ++ttx)
//        {
//            for (int n = 0; n < dst.channels(); ++n){
//                double vtxt = subStr[n] / 255.0;
//                int cvv = vtxt * color.val[n] + (1 - vtxt) * subImg[n];
//                subImg[n] = cvv > 255 ? 255 : (cvv < 0 ? 0 : cvv);
//            }

//            subStr += 3;
//            subImg += dst.channels();
//        }
//    }

//    SelectObject(hDC, hOldBmp);
//    SelectObject(hDC, hOldFont);
//    DeleteObject(hf);
//    DeleteObject(hBmp);
//    DeleteDC(hDC);
}


