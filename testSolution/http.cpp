#include "stdafx.h"
#include "http.h"
#include "utils.h"
#pragma comment(lib, "Wininet.lib")
#pragma warning(disable:4996)
#include <tchar.h>

using namespace std;
using namespace kmvc;


CWininetHttp::CWininetHttp(void):m_hSession(NULL),m_hConnect(NULL),m_hRequest(NULL)
{
}

CWininetHttp::~CWininetHttp(void)
{
    Release();
}

//  通过HTTP请求：Get或Post进行请求 [3/14/2017/shike]
const ERROR_CODE CWininetHttp::httpRequestInfo(const std::string& lpUrl,
                                                std::string type,
                                                const char* header,
                                                const int headerSize,
                                                const char* content,
                                                const int contentSize,
                                                std::string &response)
{
    string strRet = "";
    try
    {
        if ( lpUrl.empty())
        {
            return ERROR_HPIF_URL_ILLEGAL;
        }
        Release();
        char appname[] = "Http-connect";
        const size_t cSize = strlen(appname) + 1;
        WCHAR * wc = new WCHAR[cSize];
		mbstowcs(wc, appname, cSize);
        m_hSession = InternetOpen(wc, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, NULL);    //局部
        delete[] wc;
        if ( NULL == m_hSession )
        {
            return ERROR_HPIF_INIT_FAILD;
        }

        INTERNET_PORT port    = INTERNET_DEFAULT_HTTP_PORT;
        string strHostName = "";
        string strPageName = "";

        parseURLWeb(lpUrl, strHostName, strPageName, port);

        m_hConnect = InternetConnectA(m_hSession, strHostName.c_str(), port, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL);

        if ( NULL == m_hConnect )
        {
            return ERROR_HPIF_CONNECT_FAILD;
        }

        m_hRequest = HttpOpenRequestA(m_hConnect,type.c_str(), strPageName.c_str(),"HTTP/1.1", NULL, NULL, INTERNET_FLAG_RELOAD, NULL);
        if ( NULL == m_hRequest )
        {
            return ERROR_HPIF_INIT_FAILD;
        }

        BOOL bRet = FALSE;
        if ( type == "GET" )
        {
            bRet = HttpSendRequestA(m_hRequest,header,headerSize,NULL, 0);
        }
        else
        {
            bRet = HttpSendRequestA(m_hRequest,header,headerSize,(LPVOID)content, contentSize);
        }
        if ( !bRet )
        {
            return ERROR_HPIF_SEND_FAILD;
        }

        char szBuffer[READ_BUFFER_SIZE + 1] = {0};
        DWORD dwReadSize = READ_BUFFER_SIZE;
        if ( !HttpQueryInfoA(m_hRequest, HTTP_QUERY_RAW_HEADERS, szBuffer, &dwReadSize, NULL) )
        {
            return ERROR_HPIF_QUERRY_FAILD;
        }
        if ( NULL != strstr(szBuffer, "404") )
        {
            return ERROR_HPIF_404;
        }

        while( true )
        {
            bRet = InternetReadFile(m_hRequest, szBuffer, READ_BUFFER_SIZE, &dwReadSize);
            if ( !bRet || (0 == dwReadSize) )
            {
                break;
            }
            szBuffer[dwReadSize]='\0';
            strRet.append(szBuffer);
        }
    }
    catch(int)
    {
        return ERROR_HPIF_EXCEPTION;
    }
    response = std::move(strRet);
    return ERROR_HPIF_NO_ERROR;
}

const ERROR_CODE CWininetHttp::Login(const string &url,
                                     const string username,
                                     const string password,
                                     string &token)
{
    string header = "Content-Type: application/json\r\n";
    string content = "{\"name\":\"" + username + "\",\"password\":\""+password+"\"}";
    string response = "";
    ERROR_CODE code = httpRequestInfo(url,"POST",header.c_str(),(int)header.length(),
									content.c_str(),(int)content.length(),response);
    token = response;
    return code;
}


// 关闭句柄 [3/14/2017/shike]
void CWininetHttp::Release()
{
    ReleaseHandle(m_hRequest);
    ReleaseHandle(m_hConnect);
    ReleaseHandle(m_hSession);
}


// 释放句柄 [3/14/2017/shike]
void CWininetHttp::ReleaseHandle( HINTERNET& hInternet )
{
    if (hInternet)
    {
        InternetCloseHandle(hInternet);
        hInternet = NULL;
    }
}



const ERROR_CODE CWininetHttp::imageUploadRequest(const string &lpUrl,
                                                  const string& picPath,
                                                  const map<string,string>& keywords,
                                                  string &response)
{
    response = "";
    ERROR_CODE code = ERROR_HPIF_NO_ERROR;
    char *pic;
	try
    {
       string imgName = getImgName(picPath);
       int pic_len=0;
       pic = readImgFile(picPath.c_str(),pic_len);
       code = imageUploadRequest(lpUrl,pic,pic_len,imgName.c_str(),keywords,response);

    }
    catch(int)
    {
        code = ERROR_HPIF_EXCEPTION;
    }
    delete[] pic;
    pic = NULL;
    return code;
}

const ERROR_CODE CWininetHttp::imageUploadRequest(const string &lpUrl,
                                                  const char* pic,
                                                  const int pic_len,
                                                  const char* pic_name,
                                                  const map<string,string>& keywords,
                                                  string &response)
{
    response = "";
    ERROR_CODE code = ERROR_HPIF_NO_ERROR;
    int maxsize = 1024*1024*5;
    char* contentBuffer = new char[maxsize];
    char* beginPos;
    try
    {
        memset(contentBuffer,0,maxsize);
        int contentLen = 0;

        string boundary = "7d86d16250370";

        //header
        string header = "Content-Type: multipart/form-data; boundary="+boundary+"\r\n";

        //content
        //image content
        string imgName(pic_name);
        string img_content = "";
        img_content += "--"+boundary+"\r\n";
        img_content += "Content-Disposition: form-data; name=\"file\"; filename=\"";
        img_content += imgName;
        img_content += "\"\r\n";
        img_content += "Content-Type: image/jpeg \r\n\r\n";
        beginPos = bufcat(contentBuffer,img_content.c_str(),(int)img_content.length());
        contentLen += (int)img_content.length();

        string pic_end = "\r\n";
        beginPos = bufcat(beginPos,pic,pic_len);
        beginPos = bufcat(beginPos,pic_end.c_str(),(int)pic_end.length());
        contentLen += pic_len;
        contentLen += (int)pic_end.length();

        //keyword content
        map<string,string>::const_iterator iter;
        for(iter = keywords.begin();iter!= keywords.end();iter++)
        {
            string key_word = "";
            key_word += "--"+boundary+"\r\n";
            key_word += "Content-Disposition: form-data; name=\"";
            key_word += iter->first;
            key_word += "\"\r\n\r\n";
            key_word += iter->second;
            key_word += "\r\n";
            beginPos = bufcat(beginPos,key_word.c_str(),(int)key_word.length());
            contentLen += (int)key_word.length();
        }

        string contentEnd = ("--"+boundary+"--\r\n");
        beginPos = bufcat(beginPos,contentEnd.c_str(),(int)contentEnd.length());
        contentLen += (int)contentEnd.length();

        char temp[64] = { 0 };
        sprintf(temp, "Content-Length: %d\r\n\r\n",contentLen);
        header += temp;
        std::string str_http_request;
        str_http_request.append(header);

        code = httpRequestInfo(lpUrl,"POST",header.c_str(),(int)header.length(),
								contentBuffer,contentLen,response);
    }
    catch(int)
    {
        code = ERROR_HPIF_EXCEPTION;
    }
    delete[] contentBuffer;
    contentBuffer = NULL;
    return code;
}




