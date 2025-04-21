

#ifndef __ICATEVENTDATA_H__
#define __ICATEVENTDATA_H__


#include <string>


namespace cat
{


///< 网络协议
namespace NetProtocol
{


static const std::string HSerialNet = "Serial";
static const std::string HTcpNet = "TCP";
static const std::string HTcpServiceNet = "TCP Service";
static const std::string HUdpNet = "UDP";


}


///< 解析协议
namespace Protocol
{


static const std::string HOldSDKProtocol        = "SDK1.0";
static const std::string HSDKProtocol           = "SDK2.0";
static const std::string HSerialSDKProtocol     = "Serial SDK";
static const std::string HHexRawProtocol        = "Hex Raw";
static const std::string HRawStringProtocol     = "Raw String";
static const std::string HLcdSdkProtocol        = "LCD SDK";
static const std::string HUdpFindDeviceProtocol = "Udp Find Device";
static const std::string HHttpProtocol          = "Http";


}

///< 波特率
enum eBaudRate {
    kBaud1200 = 1200,
    kBaud2400 = 2400,
    kBaud4800 = 4800,
    kBaud9600 = 9600,
    kBaud19200 = 19200,
    kBaud38400 = 38400,
    kBaud57600 = 57600,
    kBaud115200 = 115200,
};

///< 数据位
enum eDataBits {
    kData5 = 5,
    kData6 = 6,
    kData7 = 7,
    kData8 = 8,
};

///< 校验位
enum eParity {
    kNoParity = 0,
    kOddParity = 1,
    kEvenParity = 2,
    kMarkParity = 3,
    kSpaceParity = 4,
};

///< 停止位
enum eStopBits {
    kOneStop = 0,
    kOneAndHalfStop = 1,
    kTwoStop = 2,
};

///< UDP类型
enum eUdpSendType {
    kSendUnicast,   ///< 单播
    kSendMulticast, ///< 组播
    kSendBroadcast, ///< 广播
};

}

#endif // __ICATEVENTDATA_H__
