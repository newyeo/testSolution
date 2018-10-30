#ifndef ERRORCODE_H
#define ERRORCODE_H

namespace kmvc {

//! error code
typedef enum {

    // success
    ERROR_NO_ERROR 					= 0, 		// Success

    // localize failed
    ERROR_LOCALIZE_DATE_FAILED		= 1,		// Localize Date Roi Failed
    ERROR_LOCALIZE_NO_FAILED 		= 2,		// Localize No. Roi Failed
    ERROR_LOCALIZE_NUM_FAILED 		= 3,		// Localize Num. Roi Failed
    ERROR_LOCALIZE_MONEY_FAILED		= 4,		// Localize Money Roi Failed

    // segment failed
    ERROR_SEGMENT_DATE_FAILED 		= 5,		// Segment Date Roi Failed
    ERROR_SEGMENT_NO_FAILED 		= 6,		// Segment No. Roi Failed
    ERROR_SEGMENT_NUM_FAILED 		= 7,		// Segment Num. Roi Failed
    ERROR_SEGMENT_MONEY_FAILED 		= 8,		// Segment Money Roi Failed

    // recognize failed
    ERROR_RECOGNIZE_DATE_FAILED 	= 9, 		// Recognize Date Character Failed
    ERROR_RECOGNIZE_NO_FAILED		= 10, 		// Recognize No. Character Failed
    ERROR_RECOGNIZE_NUM_FAILED 		= 11, 		// Recognize Num. Character Failed
    ERROR_RECOGNIZE_MONEY_FAILED	= 12, 		// Recognize Money Character Failed.

    ERROR_CALIBRATE_FAILED 			= 13,		// Calibrate Bill Image Failed.

    ERROR_BILL_WATCHER_FAILED 		= 14, 		// Detect Bill Failed.


    //http code
    //success
    ERROR_HPIF_NO_ERROR              = 100,
    ERROR_HPIF_INIT_FAILD            = 101,
    ERROR_HPIF_CONNECT_FAILD         = 102,
    ERROR_HPIF_SEND_FAILD            = 103,
    ERROR_HPIF_QUERRY_FAILD          = 104,
    ERROR_HPIF_404                   = 105,
    ERROR_HPIF_URL_ILLEGAL           = 106,
    ERROR_HPIF_EXCEPTION             = 107,

    //camera error
    ERROR_CAMERA_NO_ERROR            = 150,
    ERROR_CAMERA_GRAB_ERROR          = 151

} ERROR_CODE;

enum HttpInterfaceError
{
    Hir_Success = 0,        //成功
    Hir_InitErr,            //初始化失败
    Hir_ConnectErr,            //连接HTTP服务器失败
    Hir_SendErr,            //发送请求失败
    Hir_QueryErr,            //查询HTTP请求头失败
    Hir_404,                //页面不存在
    Hir_IllegalUrl,            //无效的URL
    Hir_CreateFileErr,        //创建文件失败
    Hir_DownloadErr,        //下载失败
    Hir_QueryIPErr,            //获取域名对应的地址失败
    Hir_SocketErr,            //套接字错误
    Hir_UserCancel,            //用户取消下载
    Hir_BufferErr,            //文件太大，缓冲区不足
    Hir_HeaderErr,            //HTTP请求头错误
    Hir_ParamErr,            //参数错误，空指针，空字符
    Hir_UnknowErr,
};


} // namespace CVBW
#endif // ERRORCODE_H
