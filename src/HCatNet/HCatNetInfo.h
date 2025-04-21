#ifndef HCATNETINFO_H
#define HCATNETINFO_H

#if defined(_MSC_VER) || defined(WIN64) || defined(_WIN64) || defined(__WIN64__) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define H_WIN
#  define Q_DECL_EXPORT __declspec(dllexport)
#  define Q_DECL_IMPORT __declspec(dllimport)
#else
#define H_LINUX
#  define Q_DECL_EXPORT     __attribute__((visibility("default")))
#  define Q_DECL_IMPORT     __attribute__((visibility("default")))
#endif

// 使用静态库时, 不需要导出文件
#ifdef STATIC_IMPORT
#ifdef Q_DECL_IMPORT
#undef Q_DECL_IMPORT
#define Q_DECL_IMPORT
#endif
#endif

#if defined(HCATNET_LIBRARY)
#  define HCATNET_EXPORT Q_DECL_EXPORT
#else
#  define HCATNET_EXPORT Q_DECL_IMPORT
#endif



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


#include <string>
#include <list>


namespace cat
{


///< LCD协议的发送文件类型
namespace HLcdSDK {

enum eFileType {
    kImage              = 1,    ///< 图片
    kVideo              = 2,    ///< 视频
    kMusic              = 3,    ///< 音乐
    kResource           = 4,    ///< 其他节目资源文件
    kFont               = 5,    ///< 字体文件
    kApk                = 6,    ///< APK
    kFirmware           = 7,    ///< 固件
    kScreenParameters   = 8,    ///< 屏参
};

}

namespace HSDK {

enum eFileType {
    kImageFile          = 0,    ///< 图片
    kVideoFile          = 1,    ///< 视频
    kFont               = 2,    ///< 字体
    kFireware           = 3,    ///< 固件
    kFPGAConfig         = 4,
    kSettingCofnig      = 5,
    KProjectResources   = 6,    ///< 资源文件
    kData               = 7,
    kTemp               = 8,
    kTempImageFile      = 128,  ///< 临时图片文件
    kTempVideoFile      = 129,  ///< 临时视频文件
};

}

///< 会话发送文件的结构参数
struct HSendFile
{
    std::string filePath;   ///< 文件路径
    std::string uuid;       ///< lcd协议需要的会话id, 默认空即可
    std::string md5;        ///< 没有会在内部进行计算
    std::string fileName;   ///< 部分协议的文件名不是真实文件名, 而是需要自行设置
    int type;               ///< 文件类型
    HSendFile() : type(0) {}
    HSendFile(const std::string &filePath_, int type_) : filePath(filePath_), type(type_) {}
    HSendFile(const std::string &filePath_, const std::string &uuid_, int type_) : filePath(filePath_), uuid(uuid_), type(type_) {}
    HSendFile(const std::string &filePath_, const std::string &uuid_, const std::string &md5_, int type_) : filePath(filePath_), uuid(uuid_), md5(md5_), type(type_) {}
    HSendFile(const std::string &filePath_, const std::string &uuid_, const std::string &md5_, const std::string &fileName_, int type_) : filePath(filePath_), uuid(uuid_), md5(md5_), fileName(fileName_), type(type_) {}
};

///< 传递发送文件类型
using SendFileListType = std::list<HSendFile>;


///< UDP探测协议数据
struct HProbeInfo
{
    enum eType {
        kSDK1_0,
        kSDK2_0,
    };
    std::string id;
    std::string ip;
    eType type;

    HProbeInfo() : type(kSDK1_0) {}
    HProbeInfo(const std::string &id_, const std::string &ip_, eType type_) : id(id_), ip(ip_), type(type_) {}
    HProbeInfo(std::string &&id_, std::string &&ip_, eType type_) : id(std::move(id_)), ip(std::move(ip_)), type(type_) {}
};


}


#endif // HCATNETINFO_H
