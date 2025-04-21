

#ifndef __HCatNet_TOOL_H__
#define __HCatNet_TOOL_H__


#include <HCatNetInfo.h>
#include <string>


namespace HTool
{


std::string HCATNET_EXPORT iconv(huint16 value);
std::string HCATNET_EXPORT iconv(huint32 value);

huint16 HCATNET_EXPORT iconvu16(const std::string &data);
huint16 HCATNET_EXPORT iconvu16(const char *data);
huint32 HCATNET_EXPORT iconvu32(const std::string &data);
huint32 HCATNET_EXPORT iconvu32(const char *data);

std::string HCATNET_EXPORT ToHex(const std::string &data, const std::string &separate = std::string());
std::string HCATNET_EXPORT FromHex(const std::string &data);

///< 兼容性错误CRC32校验, 用于错误性传感器crc校验
huint32 HCATNET_EXPORT SensorCrc32(const char *data, hint32 len);
huint32 HCATNET_EXPORT crc32(const char *data, hint32 len);

huint16 HCATNET_EXPORT ModbusCrc16(const char *buf, hint32 len);

///< 要求filePath是utf8编码, exit为true时是退出
std::string HCATNET_EXPORT MD5_U8(const std::string &filePath, bool *exit = nullptr);

///< 生成uuid
std::string HCATNET_EXPORT Uuid();


}

#endif // __HCatNet_TOOL_H__
