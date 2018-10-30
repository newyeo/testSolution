#pragma once
#include <iostream>
#include <windows.h>
#include <wininet.h>
#include"errorcode.h"
#include<map>


#define READ_BUFFER_SIZE        4096


class CWininetHttp
{
public:
    CWininetHttp(void);
    ~CWininetHttp(void);

public:

    const kmvc::ERROR_CODE httpRequestInfo( const std::string& strUrl,
                                 std::string type,
                                 const char* header,
                                 const int headerSize,
                                 const char* content,
                                 const int contentSize,
                                 std::string &response);

    const kmvc::ERROR_CODE imageUploadRequest( const std::string& strUrl,
                                 const std::string& picPath,
                                 const std::map<std::string,std::string>& keywords,
                                 std::string& response);

    const kmvc::ERROR_CODE imageUploadRequest( const std::string& strUrl,
                                 const char* pic,
                                 const int pic_len,
                                 const char* pic_name,
                                 const std::map<std::string,std::string>& keywords,
                                 std::string& response);

    const kmvc::ERROR_CODE Login(const std::string& url,
                                 const std::string username,
                                 const std::string password,
                                 std::string &token);

protected:

    void Release();

    void ReleaseHandle( HINTERNET& hInternet );



private:
    HINTERNET            m_hSession;
    HINTERNET            m_hConnect;
    HINTERNET            m_hRequest;
};
