

#ifndef __ICAPI_H__
#define __ICAPI_H__


#if defined(_MSC_VER) || defined(WIN64) || defined(_WIN64) || defined(__WIN64__) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#  define Q_DECL_EXPORT __declspec(dllexport)
#  define Q_DECL_IMPORT __declspec(dllimport)
#  define DLL_CALL      __cdecl
#  define H_WIN
#else
#  define Q_DECL_EXPORT     __attribute__((visibility("default")))
#  define Q_DECL_IMPORT     __attribute__((visibility("default")))
#  define DLL_CALL
// __attribute__((__cdecl__))
#  define H_LINUX
#endif

// 使用静态库时, 不需要导出文件
#ifdef STATIC_IMPORT
#ifdef Q_DECL_IMPORT
#undef Q_DECL_IMPORT
#define Q_DECL_IMPORT
#endif
#endif

#if defined(HCATNET_LIBRARY)
#  define BASE_API Q_DECL_EXPORT
#else
#  define BASE_API Q_DECL_IMPORT
#endif


#ifdef HCATNET_LIBRARY
namespace cat {
typedef class ISession HSession;
typedef class ICatEventBase HEventCore;
}
using cat::HEventCore;
using cat::HSession;
#else
typedef void HSession;
typedef void HEventCore;
#endif


#define HBool   int
#define HTrue   1
#define HFalse  0

typedef unsigned long long      huint64;
typedef unsigned int            huint32;
typedef unsigned short          huint16;
typedef unsigned char           huint8;

typedef long long               hint64;
typedef int                     hint32;
typedef short                   hint16;
typedef char                    hint8;
typedef float                   hfloat;
typedef double                  hdouble;

#ifdef __cplusplus
extern "C" {
#endif

///< 这是提供C接口的头文件, 注解已写到注释, 可查看使用例子. 详情请看接口文档
///< 字符串数据编码: UTF-8


///< 构建和释放事件核心, 常规情况一个事件核心单例即可
BASE_API HEventCore * DLL_CALL CreateEventCore();
BASE_API void DLL_CALL FreeEventCore(const HEventCore *core);


///< Exec阻塞, 将当前进程转换成事件进程.
///< Run不阻塞, 内部自行开一个线程当事件线程.
///< Quit通知退出事件循环.
BASE_API HBool DLL_CALL Exec(const HEventCore *core);
BASE_API HBool DLL_CALL Run(const HEventCore *core);
BASE_API void DLL_CALL Quit(const HEventCore *core);


///< 相关设置会话的属性类型
typedef enum {
    kReadyReadFunc,                 ///< 接收到数据的回调
    kReadyReadUserData,             ///< 接收到数据的用户数据
    kNetStatusFunc,                 ///< 网络状态回调
    kNetStatusUserData,             ///< 网络状态的用户数据
    kUploadFileProgressFunc,        ///< 上传文件进度
    kUploadFileProgressUserData,    ///< 上传文件进度的用户数据
    kErrorCodeFunc,                 ///< 返回的错误码
    kErrorCodeUserData,             ///< 返回错误码的用户数据
    kDeviceInfoFunc,                ///< 获取的设备信息
    kDeviceInfoUserData,            ///< 获取的设备信息的用户数据
    kNewConnectFunc,                ///< 新客户端连接过来的请求
    kNewConnectUserData,            ///< 新客户端连接过来的请求的用户数据
    kDebugLogFunc,                  ///< 协议日志信息
    kDebugLogUserData,              ///< 协议日志信息的用户数据
} eSessionType;


typedef enum {
    kDisconnect,    ///< 网络状态断开
    kConnect        ///< 网络状态连接
} eNetStatus;

typedef enum {
    kUploading,     ///< 文件上传中
    kFaild,         ///< 文件上传失败
    kSuccess,       ///< 文件上传成功
} eUploadFileStatus;

typedef enum {
    kSDK2,          ///< SDK2.0, 会话会转变为Tcp
    kSDK2Service,   ///< SDK2.0服务端, 会话会转换为Tcp服务
    kDetectDevice,  ///< 探测设备, 会话会转变为Udp
} eNetProtocol;

///< 通用回调
typedef void (*NetStatusCallBack)(HSession *currSession, eNetStatus status, void *userData);
typedef void (*LogOutCallBack)(HSession *currSession, const char *log, huint32 len, void *userData);

///< Tcp回调
typedef void (*ReadyReadCallBack)(HSession *currSession, const char *data, huint32 len, void *userData);
typedef void (*UploadFileProgressCallBack)(HSession *currSession, const char *filePath, hint64 sendSize, hint64 fileSize, eUploadFileStatus status, void *userData);
typedef void (*ErrorCodeCallBack)(HSession *currSession, int status, void *userData);

///< Tcp服务端回调
typedef void (*NewConnect)(HSession *currSession, HSession *newSession, void *userData);

///< Udp回调
typedef void (*DeviceInfoCallBack)(HSession *currSession, const char *id, huint32 idLen, const char *ip, huint32 ipLen, const char *readData, huint32 dataLen, void *userData);

///< 构建, 释放和设置网络会话
BASE_API HSession * DLL_CALL CreateNetSession(const HEventCore *core, eNetProtocol protocol);
BASE_API void DLL_CALL FreeNetSession(HSession *session);
BASE_API HBool DLL_CALL SetNetSession(HSession *session, int type, void *data);


///< 网络控制接口
BASE_API HBool DLL_CALL Isconnect(const HSession *session);
BASE_API HBool DLL_CALL Connect(HSession *session, const char *ip, int port);
BASE_API void DLL_CALL Disconnect(HSession *session);

///< 发送接口
BASE_API HBool DLL_CALL SendSDK(HSession *session, const char *data, huint32 len);
BASE_API HBool DLL_CALL SendFile(HSession *session, const char *filePath, int type);


#ifdef __cplusplus
}
#endif


#endif // __ICAPI_H__
