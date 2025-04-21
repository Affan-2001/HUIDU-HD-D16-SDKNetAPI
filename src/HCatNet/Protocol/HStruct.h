
#ifndef __HSTRUCT_H__
#define __HSTRUCT_H__


#include <HCatNetInfo.h>
#include <Tool/HTool.h>
#include <HCatAny.h>
#include <string>
#include <map>


#define MD5_LENGHT  (32)    ///< md5长度
#define ID_LENGTH   (15)    ///< 最长设备id长度
#define MAC_LENGHT  (6)     ///< MAC地址长度.


namespace cat
{


namespace HCmdType {


enum eHCmdType
{
    kTcpHeartbeatAsk        = 0x005f,   ///< TCP心跳包请求
    kTcpHeartbeatAnswer     = 0x0060,   ///< TCP心跳包反馈
    kSearchDeviceAsk        = 0x1001,   ///< 搜索设备请求
    kSearchDeviceAnswer     = 0x1002,   ///< 搜索设备应答
    kErrorAnswer            = 0x2000,   ///< 出错反馈

    kLCDServiceAsk          = 0x2101,   ///< LCD服务协商请求
    kLCDServiceAnswer       = 0x2102,   ///< LCD服务协商应答
    kLCDMsgAsk              = 0x2103,   ///< LCD 消息指令请求
    kLCDMsgAnswer           = 0x2104,   ///< LCD 消息指令应答

    kSDKServiceAsk          = 0x2001,   ///< 版本协商请求
    kSDKServiceAnswer       = 0x2002,   ///< 版本协商应答
    kSDKCmdAsk              = 0x2003,   ///< sdk命令请求
    kSDKCmdAnswer           = 0x2004,   ///< sdk命令反馈
    kGPSInfoAnswer          = 0x3007,   ///< gps信息应答
    kFileStartAsk           = 0x8001,   ///< 文件开始传输请求
    kFileStartAnswer        = 0x8002,   ///< 文件开始传输应答
    kFileContentAsk         = 0x8003,   ///< 携带文件内容的请求
    kFileContentAnswer      = 0x8004,   ///< 写文件内容的应答
    kFileEndAsk             = 0x8005,   ///< 文件结束传输请求
    kFileEndAnswer          = 0x8006,   ///< 文件结束传输应答
    kReadFileAsk            = 0x8007,   ///< 回读文件请求
    kReadFileAnswer         = 0x8008,   ///< 回读文件应答

    kFileListStartAsk       = 0x80FC,	///< 文件列表开始传输请求
    kFileListStartAnswer	= 0x80FD,	///< 文件列表开始传输应答
    kFileListEndAsk         = 0x80FE,	///< 文件列表传输完成请求
    kFileListEndAnswer      = 0x80FF,	///< 文件列表传输完成应答
};
std::string HCATNET_EXPORT HCmdTypeStr(hint32 cmd);


}


namespace HOldCmdType
{

enum eHOldCmdType
{
    kSearchAsk                  = 1,    ///< 探测包
    kSearchAnswer               = 2,    ///< 探测反馈包
    kDeviceInfoAsk              = 3,    ///< 获取设备信息
    kDeviceInfoAnswer           = 4,    ///< 设备信息反馈
    kUpdateDeviceInfoAsk        = 5,    ///< 更新设备信息
    kUpdateDeviceInfoAnswer     = 6,    ///< 更新设备信息反馈
    kSetNetAddrAsk              = 7,    ///< 设置网络地址信息.
    kSetNetAddrAnswer           = 8,    ///< 设置网络地址信息应答
    kGetNetAddrAsk              = 9,    ///< 获取网络地址信息请求
    kGetNetAddrAnswer           = 10,   ///< 获取网络地址信息应答
    kVersionAsk                 = 11,   ///< 发送/获取版本号
    kVersionAnswer              = 12,   ///< 版本号应答.
    kUpdateProjectAsk           = 13,   ///< 节目更新请求
    kUpdateProjectAnswer        = 14,   ///< 节目更新反馈
    kFreeSpaceSizeAsk           = 15,   ///< 获取可存放节目工程文件的大小
    kFreeSpaceSizeAnswer        = 16,   ///< 可用存储空间反馈
    kFileListAsk                = 17,   ///< 获取完整的节目工程文件列表
    kFileListAnswer             = 18,   ///< 完整的节目工程文件列表
    kImcompleteFileAsk          = 19,   ///< 获取不完整的文件名
    kImcompleteFileAnswer       = 20,   ///< 不完整的文件名反馈
    kRemoveFileListAsk          = 21,   ///< 删除文件列表请求
    kRemoveFileListAnswer       = 22,   ///< 删除文件列表反馈
    kOpenFileAsk                = 23,   ///< 打开文件请求
    kOpenFileAnswer             = 24,   ///< 打开文件反馈
    kFileContentAsk             = 25,   ///< 发送文件内容
    kFileContentAnswer          = 26,   ///< 文件内容接收反馈
    kCloseFileAsk               = 27,   ///< 关闭当前传输文件的请求
    kCloseFileAnswer            = 28,   ///< 关闭当前传输文件的反馈
    kTransEndAsk                = 29,   ///< 传输完成请求
    kRecvEndAnswer              = 30,   ///< 接收完成应答
    kUpdateProjectQuit          = 31,   ///< 退出节目更新
    kProjectQuitAnswer          = 32,   ///< 更新节目退出应答
    kFPGASettingInAsk           = 33,   ///< 进入FPGA参数设置
    kFPGASettingInAnswer        = 34,   ///< 进入FPGA参数设置反馈
    kFPGASettingOutAsk          = 35,   ///< 退出FPGA参数设置
    kFPGASettingOutAnswer       = 36,   ///< 退出FPGA参数设置反馈
    kFPGAParamSetAsk            = 37,   ///< FPGA参数设置请求
    kFPGAParamSetAnswer         = 38,   ///< FPGA参数设置反馈
    kFPGASetCMDAsk              = 39,   ///< FPGA参数设置命令请求
    kFPGASetCMDAnswer           = 40,   ///< FPGA参数设置命令反馈
    kBootScreenInAsk            = 41,   ///< 进入开机画面设置
    kBootScreenInAnswer         = 42,   ///< 进入开机画面设置反馈
    kBootScreenOutAsk           = 43,   ///< 退出开机画面设置
    kBootScreenOutAnswer        = 44,   ///< 退出开机画面设置反馈
    kRemoveBootScreenAsk        = 45,   ///< 删除开机画面
    kRemoveBootScreenAnswer     = 46,   ///< 删除开机画面反馈
    kLightSetInAsk              = 47,   ///< 进入亮度设置
    kLightSetInAnswer           = 48,   ///< 进入亮度设置反馈
    kLightSetOutAsk             = 49,   ///< 退出亮度设置
    kLightSetOutAnswer          = 50,   ///< 退出亮度设置反馈
    kLightFileAsk               = 51,   ///< 亮度文件发送/回读
    kLightFileAnswer            = 52,   ///< 亮度文件发送/回读反馈
    kTimeSetInAsk               = 53,   ///< 进入时间设置
    kTimeSetInAnswer            = 54,   ///< 进入时间设置反馈
    kTimeSetOutAsk              = 55,   ///< 退出时间设置
    kTimeSetOutAnswer           = 56,   ///< 退出时间设置反馈
    kSetTimeAsk                 = 57,   ///< 设置时间
    kSetTimeAnswer              = 58,   ///< 时间设置反馈
    kGetTimeAsk                 = 59,   ///< 获取时间
    kGetTimeAnswer              = 60,   ///< 获取时间反馈
    kScreenTestInAsk            = 61,   ///< 进入屏幕测试
    kScreenTestInAnswer         = 62,   ///< 进入屏幕测试反馈
    kScreenTestOutAsk           = 63,   ///< 退出屏幕测试
    kScreenTestOutAnswer        = 64,   ///< 退出屏幕测试反馈
    kScreenTestCMDAsk           = 65,   ///< 屏幕测试绘图命令
    kScreenTestCMDAnswer        = 66,   ///< 屏幕测试绘图反馈
    kBoxPlayerInAsk             = 67,   ///< BoxPlayer标识请求
    kBoxPlayerInAnswer          = 68,   ///< BoxPlayer标识请求反馈
    kBoxPlayerOutAsk            = 69,   ///< BoxPlayer退出请求
    kBoxPlayerOutAnswer         = 70,   ///< BoxPlayer退出请求反馈
    kBoxPlayerPlayAsk           = 71,   ///< 通知BoxPlayer进行播放
    kBoxPlayerPlayAnswer        = 72,   ///< BoxPlayer播放反馈
    kBoxScreenTestDataAsk       = 73,   ///< 屏幕测试数据请求
    kBoxScreenTestDataAnswer    = 74,   ///< 屏幕测试数据请求反馈
    kBoxNetworkErrorAsk         = 75,   ///< HDPlayer与BoxIOServices网络通信异常
    kBoxNetworkErrorAnswer      = 76,   ///< 对kBoxNetworkErrorAsk的反馈
    kBoxPlayerStopAsk           = 77,   ///< 通知BoxPlayer停止播放请求
    kBoxPlayerStopAnswer        = 78,   ///< BoxPlayer停止播放反馈
    kBoxPlayerPlayImageAsk      = 79,   ///< 通知BoxPlayer显示特定图像请求
    kBoxPlayerPlayImageAnswer   = 80,   ///< BoxPlayer显示特定图像反馈
    kBoxPlayerStopImageAsk      = 81,   ///< 通知BoxPlayer停止显示特定图像请求
    kBoxPlayerStopImageAnswer   = 82,   ///< BoxPlayer停止显示特定图像反馈
    kUpgradeInAsk               = 83,   ///< 下位机软件升级请求
    kUpgradeInAnswer            = 84,   ///< 下位机软件升级请求反馈
    kUpgradeCMDAsk              = 85,   ///< 下位机软件升级内部命令请求
    kUpgradeCMDAnswer           = 86,   ///< 下位机软件升级内部命令请求反馈
    kUpgradeOutAsk              = 87,   ///< 退出下位机软件升级请求
    kUpgradeOutAnswer           = 88,   ///< 退出下位机软件升级请求反馈
    kScreenWidthHeightAsk       = 89,   ///< BoxPlayer通知改变宽高信息请求
    kScreenWidthHeightAsnwer    = 90,   ///< BoxIOServices对宽高信息的反馈
    kUpdateIDAsk                = 91,   ///< 修改设备ID请求
    kUpdateIDAnswer             = 92,   ///< 修改设备ID反馈
    kUpdateMACAsk               = 93,   ///< 修改设备MAC地址请求
    kUpdateMACAnswer            = 94,   ///< 修改设备MAC地址反馈
    kTcpHeartbeatPacketAsk      = 95,   ///< TCP心跳包请求
    kTcpHeartbeatPacketAnswer   = 96,   ///< TCP心跳包反馈
    kUpdateDevNameAsk           = 97,   ///< 修改设备名称请求
    kUpdateDevNameAnswer        = 98,   ///< 修改设备名称反馈
    kSensorCMD                  = 99,   ///< 传感器相关命令
    kSetServerAddrAsk           = 100,  ///< 设置服务器地址请求
    kSetServerAddrAnswer        = 101,  ///< 设置服务器地址反馈
    kGetServerAddrAsk           = 102,  ///< 请求获取服务器地址
    kGetServerAddrAnswer        = 103,  ///< 服务器地址反馈
    kDeviceRebootInAsk          = 104,  ///< 进入设备定时重启设置
    kDeviceRebootInAnswer       = 105,  ///< 进入设备定时重启设置反馈
    kRebootBackAsk              = 106,  ///< 设备定时重启回读请求
    kRebootBackAnswer           = 107,  ///< 设备定时重启回读反馈
    kRebootSetAsk               = 108,  ///< 设置设备定时重启请求
    kRebootSetAnswer            = 109,  ///< 设置设备定时重启反馈
    kDeviceRebootOutAsk         = 110,  ///< 退出设备定时重启设置
    kDeviceRebootOutAnswer      = 111,  ///< 退出设备定时重启设置反馈
    kRebootNowAsk               = 112,  ///< 立刻重启设备请求
    kRebootNowAnswer            = 113,  ///< 立刻重启设备反馈

    kKeyDefinitionsInAsk        = 114,  ///< 进入按键定义设置
    kKeyDefinitionsInAnswer     = 115,
    kKeyDefinitionsSetInfoAsk   = 116,
    kKeyDefinitionsSetInfoAnswer= 117,
    kKeyDefinitionsGetInfoAnswer= 118,
    kKeyDefinitionsGetInfoAsk   = 119,
    kKeyDefinitionsOutAsk       = 120,
    kKeyDefinitionsOutAnswer    = 121,

    kReloadKeyDefinitionsAsk    = 122,  ///< 重新加载按键定义
    kBoxPlayerSwitchingProgram  = 123,  ///< 通知BoxPlayer切换节目

    kSwitchProgramIndexAsk      = 124,
    kSwitchProgramIndexAnswer   = 125,

    kSwitchScreenInAsk          = 126,
    kSwitchScreenInAnswer       = 127,
    kSwitchScreenOutAsk         = 128,
    kSwitchScreenOutAnswer      = 129,
    kSwitchScreenFileAsk        = 130,
    kSwitchScreenFileAnswer     = 131,

    kBoxIOClientInAsk           = 132,
    kBoxIOClientInAnswer        = 133,
    kBoxIOClientOutAsk          = 134,
    kBoxIOClientOutAnswer       = 135,
    kBoxPlayerConnectChangeAsk  = 136,     ///< 通知BoxPlayer连接发生改变
    kBoxPlayerConnectChangeAnswer= 137,    ///< BoxPlayer连接改变反馈

    kLogInAsk                   = 0x101,   ///< 注册请求
    kLogInAnswer                = 0x102,   ///< 注册反馈
    kLogOutAsk                  = 0x103,   ///< 反注册请求
    kLogOutAnswer               = 0x104,   ///< 反注册反馈
    kUpdateTypeAsk              = 0x105,   ///< 节目更新类型的请求
    kUpdateTypeAnswer           = 0x106,   ///< 节目更新类型的反馈
    kProjectCompleteAsk         = 0x107,   ///< 获取节目工程完整性的请求
    kProjectCompleteAnswer      = 0x108,   ///< 节目工程完整性的反馈
    kRemoveItemListAsk          = 0x109,   ///< 删除节目ID列表请求
    kRemoveItemListAnswer       = 0x10a,   ///< 删除节目反馈
    kRemoveFinishAsk            = 0x10b,   ///< 删除完成请求
    kRemoveFinishAnswer         = 0x10c,   ///< 删除完成反馈
    kRemoveAllAsk               = 0x10d,   ///< 清空节目的请求
    kRemoveAllAnswer            = 0x10e,   ///< 清空节目的反馈
    kItemStatusInAsk            = 0x10f,   ///< 进入节目状态获取的请求
    kItemStatusInAnswer         = 0x110,   ///< 进入节目状态获取的反馈

    kItemStatusOutAsk           = 0x113,   ///< 退出节目状态获取的请求
    kItemStatusOutAnswer        = 0x114,   ///< 退出节目状态获取的反馈
    kUploadDeviceInfoAsk        = 0x115,   ///< 上传设备信息的请求
    kUploadDeviceInfoAnswer     = 0x116,   ///< 上传设备信息的反馈
    kItemNoListAsk              = 0x117,   ///< 下发节目ID号的请求
    kItemNoListAnswer           = 0x118,   ///< 下发节目ID号的反馈
    kFileMD5ListAsk             = 0x119,   ///< 下发待发送文件的MD5列表的请求
    kFileMD5ListAnswer          = 0x11a,   ///< 下发待发送文件的MD5列表的反馈
    kGetNoSendMD5ListAsk        = 0x11b,   ///< 获取不需发送的文件MD5值列表的请求
    kGetNoSendMD5ListAnswer     = 0x11c,   ///< 不需发送的文件MD5值列表的反馈
    kFunNodePositionAsk         = 0x11d,   ///< 发送区域子节点位置信息
    kFunNodePositionAnswer      = 0x11e,   ///< 接收位置信息反馈
    kMemoryDataAsk              = 0x11f,   ///< 发送内存数据信息
    kMemoryDataAnswer           = 0x120,   ///< 接收内存数据信息反馈
    kItemStatusSelectFileAsk    = 0x121,   ///< 开始下发查询文件的请求
    kItemStatusSelectFileAnswer = 0x122,   ///< 准备接收查询文件的反馈
    kItemStatusResultFileAsk    = 0x123,   ///< 回读查询结果的请求
    kItemStatusResultFileAnswer = 0x124,   ///< 上传查询结果的反馈
    kSetItemToNullAsk           = 0x125,   ///< 设置节目为null状态的请求
    kSetItemToNullAnswer        = 0x126,   ///< 设置节目为null状态的反馈
    kFunNodeUpdateAsk           = 0x127,   ///< 功能节点的数据更新的请求
    kFunNodeUpdateAnswer        = 0x128,   ///< 功能节点的数据更新的反馈
    kGetItemStatusAsk           = 0x129,   ///< 获取节目状态信息的请求
    kGetItemStatusAnswer        = 0x12a,   ///< 获取节目状态信息的反馈
    kGetParseResultAsk          = 0x12b,   ///< 获取解析结果请求
    kGetParseResultAnswer       = 0x12c,   ///< 获取解析结果反馈
    kDynamicEditInAsk           = 0x12d,   ///< 进入动态更新模式请求
    kDynamicEditInAnswer        = 0x12e,   ///< 进入动态更新模式反馈
    kParseCmdFileAsk            = 0x12f,   ///< 解析命令文件请求
    kParseCmdFileAnswer         = 0x130,   ///< 解析命令文件反馈
    kDynamicEditOutAsk          = 0x131,   ///< 退出动态更新模式请求
    kDynamicEditOutAnswer       = 0x132,   ///< 退出动态更新模式反馈
    kProgramFileReadyAsk        = 0x133,   ///< 节目文件准备就绪
    kDownloadProgramFileAnswer  = 0x134,   ///< 节目文件下载反馈
    kPppoeSetInAsk              = 0x135,   ///< 进入设置
    kPppoeSetInAnswer           = 0x136,   ///< 进入亮度设置反馈
    kPppoeSetOutAsk             = 0x137,   ///< 退出亮度设置
    kPppoeSetOutAnswer          = 0x138,   ///< 退出亮度设置反馈
    kPppoeInfoAsk               = 0x139,   ///< 亮度文件发送/回读
    kPppoeInfoAnswer            = 0x13a,   ///< 亮度文件发送/回读反馈
    kGetPppoeInfoAsk            = 0x13b,
    kGetPppoeInfoAnswer         = 0x13c,
    kPppoeOverAsk               = 0x13d,
    kPppoeOverAnswer            = 0x13e,

    kWirelessSetInAsk           = 0x13f,    ///< 上位机请求Wireless进入设置
    kWirelessSetInAnswer        = 0x140,    ///< 下位机反馈Wireless进入设置
    kWirelessInfoAsk            = 0x141,    ///< 上位机请求准备获取Wireless信息
    kWirelessInfoAnswer         = 0x142,    ///< 下位机反馈是否准备好反馈Wireless信息
    kGetWirelessInfoAnswer      = 0x143,    ///< 上位机请求接收Wireless信息
    kGetWirelessInfoAsk         = 0x144,    ///< 下位机向上位机发送Wireless信息
    kSetWirelessAsk             = 0x145,    ///< 上位机请求设置Wireless
    kSetWirelessAnswer          = 0x146,    ///< 下位机向上位机反馈设置状态

    kWirelessOverAsk            = 0x147,    ///< 结束请求
    kWirelessOverAnswer         = 0x148,    ///< 反馈结束请求
    kNetworkStatusAnswer        = 0x149,    ///< 网络状态反馈
    kNetworkConnected           = 0x14a,    ///< 网络连接上
    kNetworkUnconnected         = 0x14b,    ///< 网络断开

    kContentStartAsk            = 0x200,    ///< 内容起始请求包
    kContentStartAnswer         = 0x201,    ///< 内容起始反馈包
    kContentDataAsk             = 0x202,    ///< 内容数据包
    kContentDataAnswer          = 0x203,    ///< 内容反馈包
    kContentEndAsk              = 0x204,    ///< 内容结束请求包
    kContentEndAnswer           = 0x205,    ///< 内容结束反馈包
    kAppExternInAsk             = 0x206,    ///< 进入APP扩展协议服务请求
    kAppExternInAnswer          = 0x207,    ///< 进入APP扩展协议服务反馈
    kAppExternOutAsk            = 0x208,    ///< 退出APP扩展协议服务请求
    kAppExternOutAnswer         = 0x209,    ///< 退出APP扩展协议服务反馈

    kTcpTranInAsk               = 0x300,
    kTcpTranInAnswer            = 0x301,
    kTcpTranOutAsk              = 0x302,
    kTcpTranOutAnswer           = 0x303,
    kUdpTranAsk                 = 0x320,
    kUdpTranAnswer              = 0x321,
    kUpdateDeviceInfoExt1Ask    = 0x340,    ///< 更新设备信息
    kUpdateDeviceInfoExt1Answer = 0x341,    ///< 更新设备信息反馈

    kProgramIndexChangedAsk     = 0x400,
    kProgramIndexChangedAnswer  = 0x401,
    kBoxPlayerTimeZoneAsk       = 0x402,
    kBoxPlayerTimeZoneAnswer    = 0x403,
    kReloadAllFontsAsk          = 0x404,
    kReloadAllFontsAnswer       = 0x405,
    kGUIDSwitchProgramAsk       = 0x406,    ///< guid切换节目请求
    kGUIDSwitchProgramAnswer    = 0x407,    ///< guid切换节目反馈
    kChangeProgramAsk           = 0x408,    ///< 节目索引请求
    kChangeProgramAnswer        = 0x409,    ///< 节目索引反馈
    kTestDeviceLockerAsk        = 0x40a,    ///< 测试是否开启设备锁请求
    kTestDeviceLockerAnswer     = 0x40b,    ///< 设备是否开启设备锁反馈
    kCheckDeviceLockerAsk       = 0x40c,    ///< 设备锁匹配请求
    kCheckDeviceLockerAnswer    = 0x40d,    ///< 设备锁匹配反馈
    kHDMIInAsk                  = 0x40e,
    kHDMIInAnswer               = 0x40f,
    kClientInfoAsk              = 0x410,    ///< 客户端信息请求
    kClientInfoAnswer           = 0x411,    ///< 客户端信息反馈

    kReloadFPGAParamAsk         = 0x500,
    kReloadFPGAParamAnswer      = 0x501,
    kUpgradeFinishAsk           = 0x502,
    kUpgradeFinishAnswer        = 0x503,
    kConvertDataToOldAsk        = 0x504,    ///< 通知BoxSDK进程恢复旧版本的数据信息
    kConvertDataToOldAnswer     = 0x505,
};
std::string HCATNET_EXPORT HOldCmdTypeStr(hint32 cmd);


}


namespace HErrorCode
{


enum eHErrorCode : huint16
{
    kSuccess = 0,           ///< 正常状态
    kWriteFinish,           ///< 写文件完成
    kProcessError,          ///< 流程错误
    kVersionTooLow,         ///< 版本过低
    kDeviceOccupa,          ///< 设备被占用
    kFileOccupa,            ///< 文件被占用
    kReadFileExcessive,     ///< 回读文件用户过多
    kInvalidPacketLen,      ///< 数据包长度错误
    kInvalidParam,          ///< 无效的参数
    kNotSpaceToSave,        ///< 存储空间不够
    kCreateFileFailed,      ///< 创建文件失败
    kWriteFileFailed,       ///< 写文件失败
    kReadFileFailed,        ///< 读文件失败
    kInvalidFileData,       ///< 无效的文件数据
    kFileContentError,      ///< 文件内容出错
    kOpenFileFailed,        ///< 打开文件失败
    kSeekFileFailed,        ///< 定位文件失败
    kRenameFailed,          ///< 重命名失败
    kFileNotFound,          ///< 文件未找到
    kFileNotFinish,         ///< 文件未接收完成
    kXmlCmdTooLong,         ///< xml命令过长
    kInvalidXmlIndex,       ///< 无效的xml命令索引值
    kParseXmlFailed,        ///< 解析xml出错
    kInvalidMethod,         ///< 无效的方法名
    kMemoryFailed,          ///< 内存错误
    kSystemError,           ///< 系统错误
    kUnsupportVideo,        ///< 不支持的视频
    kNotMediaFile,          ///< 不是多媒体文件
    kParseVideoFailed,      ///< 解析视频文件失败
    kUnsupportFrameRate,    ///< 不支持的波特率
    kUnsupportResolution,   ///< 不支持的分辨率(视频)
    kUnsupportFormat,       ///< 不支持的格式(视频)
    kUnsupportDuration,     ///< 不支持的时间长度(视频)
    kDownloadFileFailed,    ///< 下载文件失败

    kDownloadingFile,       ///< 下载文件中
    kProcessing,            ///< 处理中

    kScreenNodeIsNull,      ///< 显示屏节点为null
    kNodeExist,             ///< 节点存在
    kNodeNotExist,          ///< 节点不存在
    kPluginNotExist,        ///< 插件不存在
    kCheckLicenseFailed,    ///< 校验license失败
    kNotFoundWifiModule,    ///< 未找到wifi模块
    kTestWifiUnsuccessful,  ///< 测试wifi模块未
    kRunningError,          ///< 运行错误
    kUnsupportMethod,       ///< 不支持的方法
    kInvalidGUID,           ///< 非法的guid
    kFirmwareFormatError,   ///< 固件格式错误
    kTagNotFound,           ///< 标签不存在
    kAttrNotFound,          ///< 属性不存在
    kCreateTagFailed,       ///< 创建标签失败
    kUnsupportDeviceType,   ///< 不支持的设备型号
    kPermissionDenied,      ///< 权限不足
    kPasswdTooSimple,       ///< 密码太简单
    kUsbNotInsert,          ///< usb未插入

    //非错误码
    kDelayRespond,          ///< 延迟反馈
    kShortlyReturn,         ///< 直接返回, 不进行xml转换
};
std::string HCATNET_EXPORT HErrorCodeStr(hint32 cmd);


}


#pragma pack(1)


struct HHeader
{
    huint16 len;        ///< 指令包长度
    huint16 cmd;        ///< 指令
    HHeader() : len(sizeof(HHeader)), cmd(0) {}
    HHeader(huint16 len_, huint16 cmd_) : len(len_), cmd(cmd_) {}
    explicit HHeader(const std::string &data)
        : len(HTool::iconvu16(data.data()))
        , cmd(HTool::iconvu16(data.data() + 2)){}
};

struct HTcpVersion
{
    huint16 len;        ///< 命令包长度
    huint16 cmd;        ///< 命令值
    huint32 version;    ///< 传输协议版本号
};

struct HSDKCmd
{
    huint16 len;        ///< 命令包长度
    huint16 cmd;        ///< 命令值
    huint32 total;      ///< sdk接口数据总长度
    huint32 index;      ///< sdk接口数据当前索引
    explicit HSDKCmd() : len(sizeof(HSDKCmd)), cmd(0), total(0), index(0){}
    explicit HSDKCmd(const std::string &data)
        : len(HTool::iconvu16(data.data()))
        , cmd(HTool::iconvu16(data.data() + 2))
        , total(HTool::iconvu32(data.data() + 4))
        , index(HTool::iconvu32(data.data() + 8))
    {}
};


struct HSerialSDKCmd
{
    huint16 len;        ///< 命令包长度
    huint16 cmd;        ///< 命令值
    huint32 total;      ///< sdk接口数据总长度
    huint32 index;      ///< sdk接口数据当前索引
    huint32 crc32;      ///< crc32数据值
    explicit HSerialSDKCmd() : len(sizeof(HSDKCmd)), cmd(0), total(0), index(0), crc32(0){}
    explicit HSerialSDKCmd(const std::string &data)
        : len(HTool::iconvu16(data.data()))
        , cmd(HTool::iconvu16(data.data() + 2))
        , total(HTool::iconvu32(data.data() + 4))
        , index(HTool::iconvu32(data.data() + 8))
        , crc32(HTool::iconvu32(data.data() + 12))
    {}
};


typedef struct HFileListStartAsk
{
    huint16 len;                    ///< 命令包长度
    huint16 cmd;                    ///< 命令值
    huint16 fileCount;              ///< 文件数量
    huint64 AllFileSize;            ///< 所有文件总大小(字节)
    hint8   sessionID[0];           ///< 会话id, '\0'结尾, 节目资源一般使用节目单的UUID作为会话ID
} HFileListStartAsk;

typedef struct HFileListStartAnswer
{
    huint16 len;        ///< 命令包长度
    huint16 cmd;        ///< 命令值
    huint16 status;     ///< 错误码(非0为异常)
} HFileListStartAnswer;


typedef struct HFileStartAsk
{
    huint16 len;                    ///< 命令包长度
    huint16 cmd;                    ///< 命令值
    hint8   md5[MD5_LENGHT + 1];    ///< 文件md5值
    huint64 size;                   ///< 文件大小
    hint16  type;                   ///< 文件类型 0(图片), 1(视频), 2(字体), 3(固件), 4(基本配置参数), 5(FPG配置参数)
    hint8   name[2];                ///< 文件名
} HFileStartAsk;

typedef struct HFileStartAnswer
{
    huint16 len;        ///< 命令包长度
    huint16 cmd;        ///< 命令值
    hint16  status;     ///< 错误码
    huint64 existSize;  ///< 文件在下位机存储的大小
} HFileStartAnswer;

typedef struct HFileEndAnswer
{
    huint16 len;        ///< 命令包长度
    huint16 cmd;        ///< 命令值
    huint16 status;     ///< 错误码
} HFileEndAnswer, HErrorAnswer;

typedef struct HTcpStatus
{
    HHeader head;
    huint16 status;
} HTcpStatus, HErrorStatus;


typedef struct HReadFileAsk
{
    huint16 len;        ///< 命令包长度
    huint16 cmd;        ///< 命令值
    hint8   name[2];    ///< 文件名，注：文件名后需要加’\0’
} HReadFileAsk;

typedef struct HReadFileAnswer
{
    huint16 len;                    ///< 命令包长度
    huint16 cmd;                    ///< 命令值
    hint16  status;                 ///< 错误码
    hint8   md5[MD5_LENGHT + 1];    ///< 文件md5值
    huint64 size;                   ///< 文件大小
    hint16  type;                   ///< 文件类型
} HReadFileAnswer;


struct HUDPAsk
{
    huint32 version;    ///< 版本
    huint16 cmd;        ///< 指令
    HUDPAsk() : version(0x1000007), cmd(HCmdType::kSearchDeviceAsk) {}
    HUDPAsk(huint32 version_, huint16 cmd_) : version(version_), cmd(cmd_) {}
    explicit HUDPAsk(const std::string &data)
        : version(HTool::iconvu32(data.data()))
        , cmd(HTool::iconvu16(data.data() + 4)){}
};

struct HUDPAnswer
{
    huint32 version;    ///< 版本
    huint16 cmd;        ///< 指令
    char    id[ID_LENGTH];
    hint8   data[0];
    HUDPAnswer() : version(0x1000007), cmd(HCmdType::kSearchDeviceAsk), id{0} {}
    HUDPAnswer(huint32 version_, huint16 cmd_) : version(version_), cmd(cmd_), id{0} {}
    explicit HUDPAnswer(const std::string &data)
        : version(HTool::iconvu32(data.data()))
        , cmd(HTool::iconvu16(data.data() + 4))
        , id{0}
    {}
};

/** @struct HUpdateDevInfoAsk
 *  @brief  更新设备信息.
 */
typedef struct HUpdateDevInfoAsk
{
    huint32     version;                ///< 协议版本号.
    huint16     cmd;                    ///< 更新设备信息命令.

    huint32     no;                     ///< 用来标识设备信息.

    huint8      id[ID_LENGTH];          ///< 设备ID号.
    huint8      deviceType;             ///< 设备型号类型.
    huint32     ip;                     ///< 设备IP地址.
    huint8      mac[MAC_LENGHT];        ///< 设备MAC地址.
    huint32     mask;                   ///< 设备掩码地址.
    huint32     gateway;                ///< 设备默认网关地址.
    huint32     nameServer;             ///< 设备主DNS服务器地址.
    huint32     kernelVersion;          ///< 内核版本.
    huint32     fpgaVersion;            ///< FPGA版本.
    huint32     ioServicesVersion;      ///< BoxIOServices版本.
    huint32     playerVersion;          ///< BoxPlayer版本.
    huint32     upgradeVersion;         ///< BoxUpgrade版本.
    huint32     daemonVersion;          ///< BoxDaemon版本.
    huint32     mainVersion;            ///< 主版本号.
    huint16     screenWidth;            ///< 屏幕宽.
    huint16     screenHeight;           ///< 屏幕高.
    huint8      bootScreenFlag;         ///< 开机画面有无标志.

    huint8      nameLen;                ///< 用来标示名字的长度.
    huint8      name[1];                ///< 设备名字为可变长度. 设备名字长度[1, DEVICE_NAME_MAX_LEN]
} HUpdateDevInfoAsk;

///< 更新设备信息.
typedef struct HUpdateDevInfoStrAsk
{
    std::string id;                 ///< 设备ID号.
    std::string ip;                 ///< 设备IP地址.
    std::string mac;                ///< 设备MAC地址
    std::string mask;               ///< 设备掩码地址.
    std::string gateway;            ///< 设备默认网关地址.
    std::string nameServer;         ///< 设备主DNS服务器地址.
    std::string kernelVersion;      ///< 内核版本.
    std::string fpgaVersion;        ///< FPGA版本.
    std::string ioServicesVersion;  ///< BoxIOServices版本.
    std::string playerVersion;      ///< BoxPlayer版本.
    std::string upgradeVersion;     ///< BoxUpgrade版本.
    std::string daemonVersion;      ///< BoxDaemon版本.
    std::string mainVersion;        ///< 主版本号.
    std::string name;               ///< 设备名字
    std::string xml;                ///< 附带xml信息
    huint16     screenWidth;        ///< 屏幕宽.
    huint16     screenHeight;       ///< 屏幕高.
    bool        bootScreenFlag;     ///< 开机画面有无标志.

    bool IsEmpty() const { return id.empty() || ip.empty(); }
} HUpdateDevInfoStrAsk;

///START 下面是旧SDK使用
/** @struct HContentStartAsk
 *  @brief  内容起始请求包
 */
typedef struct HContentStartAsk
{
    huint16 pLen;
    huint16 pCmd;
    huint16 inCmd;  ///< -1 init, 0 xmlTask, 1 resultRespond
    huint16 inLen;
    HContentStartAsk() : pLen(sizeof(HContentStartAsk)), pCmd(HOldCmdType::kContentStartAsk), inCmd(0), inLen(0) { }
} HContentStartAsk;


/** @struct HContentStartAnswer
 *  @brief  内容起始反馈包
 */
typedef struct HContentStartAnswer
{
    huint16 len;
    huint16 cmd;
    huint16 inCmd;  ///< -1 init, 0 xml, 1 file

    HContentStartAnswer() : len(sizeof(HContentStartAnswer)), cmd(HOldCmdType::kContentStartAnswer) { }
    explicit HContentStartAnswer(const std::string &data)
        : len(HTool::iconvu16(data.data()))
        , cmd(HTool::iconvu16(data.data() + 2))
        , inCmd(HTool::iconvu16(data.data() + 4))
    {}
} HContentStartAnswer;


/** @struct HContentDataAsk
 *  @brief  内容数据包
 */
typedef struct HContentDataAsk
{
    huint16 len;
    huint16 cmd;
    huint16 inCmd;
    huint8  data[1];
    HContentDataAsk() : len(sizeof(HContentDataAsk) - sizeof(data)), cmd(HOldCmdType::kContentDataAsk), inCmd(0) { }
} HContentDataAsk;


/** @struct HContentDataAnswer
 *  @brief  内容反馈包
 */
typedef struct HContentDataAnswer
{
    huint16 len;
    huint16 cmd;
    huint16 inCmd;
    HContentDataAnswer() : len(sizeof(HContentDataAnswer)), cmd(HOldCmdType::kContentDataAnswer) { }
} HContentDataAnswer;


/** @struct HContentEndAsk
 *  @brief  内容结束请求包
 */
typedef struct HContentEndAsk
{
    huint16 len;
    huint16 cmd;
    huint16 inCmd;
    HContentEndAsk() : len(sizeof(HContentEndAsk)), cmd(HOldCmdType::kContentEndAsk), inCmd(0) { }
} HContentEndAsk;


/** @struct HContentEndAnswer
 *  @brief  内容结束反馈包
 */
typedef struct HContentEndAnswer
{
    huint16 len;
    huint16 cmd;
    huint16 inCmd;

    HContentEndAnswer() : len(sizeof(HContentEndAnswer)) , cmd(HOldCmdType::kContentEndAnswer) {}
} HContentEndAnswer;


typedef struct HTestDeviceLockerAnswer
{
    huint16 len;
    huint16 cmd;
    hint8   enable; ///< bool值
} HTestDeviceLockerAnswer;
///END


#pragma pack()


///< 协议的回调数据
struct HReadProtocolData
{
    enum eType {
        kNone,          ///< 无数据
        kDetectDevice,  ///< 探测设备数据, 类型为 HProbeInfo 或 HUpdateDevInfoStrAsk
        kErrorStatus,   ///< 错误状态, 类型为 HErrorCode::eHErrorCode(huint16)
        kHttpResponse,  ///< http响应, 类型为 HHttpResponse
    };

    eType type;
    cat::HCatAny data;

    HReadProtocolData() : type(kNone) {}
    HReadProtocolData(eType type_, cat::HCatAny &&data_) : type(type_), data(std::move(data_)) {}
};


using AttributeTable = std::map<std::string, std::string>;
///< Http请求头
struct HHttpRequest
{
    enum eType {
        kGet,       ///< 请求指定页面
        kHead,      ///< 获取报头
        kPost,      ///< 上传资源
        kPut,       ///< 发送数据
        kDelete,    ///< 删除指定页面
        kConnect,   ///< 连接改为管道方式的代理服务器
        kOptions,   ///< 允许客户端查看服务器性能
        kTrace,     ///< 回显服务器收到请求
        kPatch,     ///< put的补充, 局部更新
    };

    eType type;
    std::string url;
    AttributeTable header;
    huint16 port;

    HHttpRequest() : type(kGet), port(80) {}
    explicit HHttpRequest(const std::string &url_) : type(kGet), url(url_), port(80) {}
    HHttpRequest(eType type_, const std::string &url_) : type(type_), url(url_), port(80) {}
};


///< Http响应头
struct HHttpResponse
{
    enum eDataStore : hint16 {
        kData,  ///< 数据传递在数据
        kFile,  ///< 数据传递在文件
    };
    hint16 status;
    eDataStore dataStore;
    AttributeTable header;

    HHttpResponse() : status(200), dataStore(kData) {}
    explicit HHttpResponse(hint16 status_) : status(status_), dataStore(kData) {}
    explicit HHttpResponse(hint16 status_, eDataStore data_) : status(status_), dataStore(data_) {}

    bool IsEmpty() const { return header.empty(); }
    void Clear() { header.clear(); }
};



} // end namespace cat;


#endif // __HSTRUCT_H__
