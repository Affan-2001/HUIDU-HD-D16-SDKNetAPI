

#include <Tool/HTool.h>
#include <Tool/md5.h>

#include <fstream>
#include <stdio.h>
#include <stdint.h>
#include <string>
#include <vector>


#ifdef H_WIN
#include <HCatTool.h>
#include <objbase.h>
#pragma comment(lib,"ole32.lib")
#elif RK_ANDROID
#include <stdlib.h>
#else
#include <uuid/uuid.h>
#endif

#define GUID_LEN        64


inline int fromHex(int c)
{
    return ((c >= '0') && (c <= '9')) ?
                int(c - '0') : ((c >= 'A') && (c <= 'F')) ?
                    int(c - 'A' + 10) : ((c >= 'a') && (c <= 'f')) ?
                        int(c - 'a' + 10) : -1;
}
inline int toHex(int c)
{
    return ((c >= 0) && (c <= 9)) ?
                int(c + '0') : ((c >= 10) && (c <= 15)) ?
                    int(c + 'a' - 10) : -1;
}

namespace HTool {


std::string iconv(huint16 value)
{
    std::string data;
    data.push_back(char(value & 0xff));
    data.push_back(char((value >> 8) & 0xff));
    return data;
}

std::string iconv(huint32 value)
{
    std::string data;
    data.push_back(char(value & 0xff));
    data.push_back(char((value >> 8) & 0xff));
    data.push_back(char((value >> 16) & 0xff));
    data.push_back(char((value >> 24) & 0xff));
    return data;
}

huint16 iconvu16(const std::string &data)
{
    return huint16(
                ( data.at(0) & 0xff )       |
                ((data.at(1) & 0xff) << 8));
}

huint32 iconvu32(const std::string &data)
{
    return huint32(
                ( data.at(0) & 0xff )       |
                ((data.at(1) & 0xff) << 8)  |
                ((data.at(2) & 0xff) << 16) |
                ((data.at(3) & 0xff) << 24));
}

huint16 iconvu16(const char *data)
{
    return huint16(
                ( data[0] & 0xFF)           |
                ((data[1] & 0xFF) << 8));
}

huint32 iconvu32(const char *data)
{
    return huint32(
                ( data[0] & 0xff )          |
                ((data[1] & 0xff) << 8)     |
                ((data[2] & 0xff) << 16)    |
            ((data[3] & 0xff) << 24));
}


std::string ToHex(const std::string &data, const std::string &separate)
{
    std::string result;
    for (std::size_t i = 0; i < data.size(); ++i) {
        unsigned char value = data[i];
        unsigned char high = toHex(value >> 4 & 0xf);
        if (high == 0xff) {
            continue;
        }

        unsigned char low = toHex(value & 0xf);
        result.push_back(high);
        result.push_back(low);


        if (separate.empty() == false) {
            result.append(separate);
        }
    }

    if (data.empty() == false && separate.empty() == false) {
        result.erase(result.size() - separate.size(), separate.size());
    }

    return result;
}

std::string FromHex(const std::string &data)
{
    std::string init = std::string((data.size() + 1) / 2, 0);
    unsigned char* result = (unsigned char*)init.data() + init.size();

    int del = 0;
    bool ri = true;
    for (int i = data.size() - 1; i >= 0; --i) {
        unsigned char ch = data.at(i);
        int tmp = fromHex(ch);
        if (-1 == tmp) {
            ++del;
            continue;
        }

        if (ri) {
            --result;
            *result = tmp;
            ri = false;
        } else {
            *result |= tmp << 4;
            ri = true;
        }
    }

    if (del != 0) {
        init.erase(0, del / 2);
    }

    return init;
}



#define ReverseUInt(value)  \
    ((value >> 24) | ((value >> 16) << 8) | ((value & 0xff00) << 8) | ((value & 0xff) << 24))
huint32 SensorCrc32(const char *data, hint32 len)
{
    if ((data == 0) || (len < 1)) {
        return 0;
    }
    const huint8 *p = reinterpret_cast<const huint8 *>(data);

    huint32 crc = 0xFFFFFFFF;
    for (int i = 0; i < len; i++) {
        huint32 byte = p[i];
        byte = ReverseUInt(byte);
        for (int j = 0; j < 8; j++) {
            if ((static_cast<hint32>(crc ^ byte)) < 0) {
                crc = (crc << 1) ^ 0x04C11DB7;
            } else {
                crc = crc << 1;
            }
            byte = byte << 1;
        }
    }

    return ReverseUInt(~crc);
}
#undef ReverseUInt


#define ReverseUInt(value)  \
    ((value >> 24) | (((value >> 16) & 0xff) << 8) | ((value & 0xff00) << 8) | ((value & 0xff) << 24))
huint32 crc32(const char *data, hint32 len)
{
    if ((data == 0) || (len < 1)) {
        return 0;
    }
    const huint8 *p = reinterpret_cast<const huint8 *>(data);

    huint32 crc = 0xFFFFFFFF;
    for (int i = 0; i < len; i++) {
        huint32 byte = p[i];
        byte = ReverseUInt(byte);
        for (int j = 0; j < 8; j++) {
            if ((static_cast<hint32>(crc ^ byte)) < 0) {
                crc = (crc << 1) ^ 0x04C11DB7;
            } else {
                crc = crc << 1;
            }
            byte = byte << 1;
        }
    }

    return ReverseUInt(~crc);
}
#undef ReverseUInt


huint16 ModbusCrc16(const char *buf, hint32 len)
{
    static const huint16 table[256] = {
        0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
        0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
        0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
        0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
        0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
        0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
        0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
        0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
        0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
        0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
        0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
        0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
        0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
        0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
        0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
        0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
        0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
        0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
        0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
        0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
        0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
        0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
        0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
        0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
        0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
        0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
        0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
        0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
        0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
        0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
        0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
        0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040 };

    huint16 crc = 0xFFFF;
    const huint8 *p = reinterpret_cast<const huint8 *>(buf);
    while(len--) {
        huint8 xorValue = (*p++) ^ crc;
        crc >>= 8;
        crc ^= table[xorValue];
    }

    return crc;
}


std::string MD5_U8(const std::string &filePath, bool *exit)
{
    std::fstream fs;
#ifdef H_WIN
    fs.open(cat::HCatTool::UTF8ToANSI(filePath), std::ios::in | std::ios::binary);
#else
    fs.open(filePath, std::ios::in | std::ios::binary);
#endif
    if (fs.is_open() == false) {
        std::cerr << "not open file: " << filePath << std::endl;
        return std::string();
    }

    MD5 hash;
    std::vector<char> buff(1024 * 1024);
    while (fs.eof() == false) {
        fs.read(buff.data(), buff.size());
        std::size_t extracted = static_cast<std::size_t>(fs.gcount());
        if (extracted == 0) {
            continue;
        }

        hash.update(buff.data(), extracted);

        if (exit && *exit) {
            return std::string();
        }
    }

    hash.finalize();
    return hash.hexdigest();
}


std::string Uuid()
{
    char buf[GUID_LEN] = { 0 };

#ifdef H_WIN
    GUID guid;
    if (CoCreateGuid(&guid)) {
        return std::move(std::string(""));
    }

    sprintf(buf,
        "%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X",
        guid.Data1, guid.Data2, guid.Data3,
        guid.Data4[0], guid.Data4[1], guid.Data4[2],
        guid.Data4[3], guid.Data4[4], guid.Data4[5],
        guid.Data4[6], guid.Data4[7]);
#elif RK_ANDROID
    typedef struct _GUID {
        unsigned long  Data1;
        unsigned short Data2;
        unsigned short Data3;
        unsigned char  Data4[ 8 ];
    } GUID;
    GUID guid;
    memset(&guid, 0, sizeof(GUID));
    for (int i = 0; i < sizeof(guid); ++i) {
        guid.Data4[i] = rand() % 255;
    }
    sprintf(buf,
        "%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X",
        guid.Data1, guid.Data2, guid.Data3,
        guid.Data4[0], guid.Data4[1], guid.Data4[2],
        guid.Data4[3], guid.Data4[4], guid.Data4[5],
        guid.Data4[6], guid.Data4[7]);
#else
    uuid_t uu;
    uuid_generate( uu );

    int index = 0;
    for (int i = 0; i < 16; i++) {
        int len = i < 15 ?
            sprintf(buf + index, "%02X-", uu[i]) :
            sprintf(buf + index, "%02X", uu[i]);
        if (len < 0) {
            return std::string("");
        }

        index += len;
    }

#endif
    return std::string(buf);
}




}
