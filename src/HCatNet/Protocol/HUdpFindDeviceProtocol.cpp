

#include <Protocol/HUdpFindDeviceProtocol.h>
#include <Protocol/HStruct.h>
#include <Net/INetBase.h>
#include <CatString.h>


#define PROTOCL_VERSION_V1      (0x1000000)       ///< 协议版本号.
#define PROTOCOL_VERSION_V1_1   (0x1000001)       ///< 协议版本号.
#define PROTOCOL_VERSION_V1_2   (0x1000002)       ///< 协议版本号.
#define PROTOCOL_VERSION_V1_3   (0x1000003)       ///< 协议版本号.
#define PROTOCOL_VERSION_V1_4   (0x1000004)       ///< 协议版本号.
#define PROTOCOL_VERSION_V1_5   (0x1000005)       ///< 协议版本号.
#define PROTOCOL_VERSION_V1_6   (0x1000006)       ///< 协议版本号.
#define PROTOCOL_VERSION_V1_7   (0x1000007)       ///< 协议版本号.

#define LOCAL_UDP_VERSION       (0x1000007)


using namespace cat;


HUdpFindDeviceProtocol::HUdpFindDeviceProtocol(const EventCoreWeakPtr &core)
    : IProtocolBase(core)
{

}

HUdpFindDeviceProtocol::~HUdpFindDeviceProtocol()
{

}

bool HUdpFindDeviceProtocol::Dispose(hint32 cmd, const HCatBuffer &data, const HCatAny &userData)
{
    switch (cmd) {
    case kInit:
    case kReset:
    case kSendFaildAsk: {
        return true;
    } break;
    case kSendFile:
    case kSendSDK:
    case kHeartBeat: {
        HUDPAsk header(LOCAL_UDP_VERSION, HOldCmdType::kSearchAsk);
        if (userData.IsVaild()) {
            header.cmd = HCmdType::kSearchDeviceAsk;
            ParseLog(fmt::format("send-{}", HCmdType::HCmdTypeStr(header.cmd)));
        } else {
            ParseLog(fmt::format("send-{}", HOldCmdType::HOldCmdTypeStr(header.cmd)));
        }
        std::string sendData;
        sendData.append(HTool::iconv(header.version));
        sendData.append(HTool::iconv(header.cmd));
        NotifyCmd(kSendData, sendData, userData);
        return true;
    } break;
    case kConnect: {
        NotifyCmd(kNotofyConnect, data, userData);
        return true;
    } break;
    case kReadData: {
        return this->ParseReadData(data, userData);
    } break;
    default:
        break;
    }

    return false;
}

std::string HUdpFindDeviceProtocol::ToIp(huint32 value)
{
    std::string ip = fmt::format("{}.{}.{}.{}", (value & 0xFF), ((value >> 8) & 0xFF), ((value >> 16) & 0xFF), ((value >> 24) & 0xFF));
    return ip;
}

void HUdpFindDeviceProtocol::ParseLog(const std::string &log)
{
#ifdef DEBUG_LOG_SIGNAL
    ICatEventApi::ForwardSignal(core_, this, DebugLog, log);
#endif
}

bool HUdpFindDeviceProtocol::ParseReadData(const HCatBuffer &data, const HCatAny &userData)
{
    if (data.Size() < sizeof(HUDPAsk)) {
        return true;
    }

    INetBase::TcpInfo info;
    if (userData.IsType<INetBase::TcpInfo>()) {
        info = userData.Cast<INetBase::TcpInfo>();
    }

    const HUDPAsk *header = reinterpret_cast<const HUDPAsk *>(data.ConstData());

    switch (header->cmd) {
    case HOldCmdType::kSearchAnswer:
    case HCmdType::kSearchDeviceAnswer: {
        if (data.Size() < sizeof(HUDPAnswer)) {
            return true;
        }

        const HUDPAnswer *answer = reinterpret_cast<const HUDPAnswer *>(data.ConstData());
        ParseLog(fmt::format("read-{}, version[{}], id[{}], ip[{}]", HCmdType::HCmdTypeStr(answer->cmd), answer->version, answer->id, info.ip));
        HProbeInfo::eType type = HProbeInfo::kSDK1_0;
        if (header->cmd == HCmdType::kSearchDeviceAnswer) {
            type = HProbeInfo::kSDK2_0;
        }
        NotifyCmd(kReadyRead, data, HReadProtocolData(HReadProtocolData::kDetectDevice, HProbeInfo(answer->id, info.ip, type)));
    } break;
    case HOldCmdType::kUpdateDeviceInfoAsk: {
        if (data.Size() < sizeof(HUpdateDevInfoAsk)) {
            return true;
        }

        const HUpdateDevInfoAsk *answer = reinterpret_cast<const HUpdateDevInfoAsk *>(data.ConstData());
        HUpdateDevInfoStrAsk ask;
        ask.id = std::string(reinterpret_cast<const char *>(answer->id));
        ask.ip = info.ip;
        ask.mac = HTool::ToHex(std::string(reinterpret_cast<const char *>(answer->mac), MAC_LENGHT), ":");
        ask.mask = ToIp(answer->mask);
        ask.gateway = ToIp(answer->gateway);
        ask.nameServer = ToIp(answer->nameServer);
        ask.kernelVersion = ToIp(answer->kernelVersion);
        ask.fpgaVersion = ToIp(answer->fpgaVersion);
        ask.ioServicesVersion = ToIp(answer->ioServicesVersion);
        ask.playerVersion = ToIp(answer->playerVersion);
        ask.upgradeVersion = ToIp(answer->upgradeVersion);
        ask.daemonVersion = ToIp(answer->daemonVersion);
        ask.mainVersion = ToIp(answer->mainVersion);
        ask.screenWidth = answer->screenWidth;
        ask.screenHeight = answer->screenHeight;
        ask.bootScreenFlag = answer->bootScreenFlag != 0;
        ask.name = std::string(reinterpret_cast<const char *>(answer->name), answer->nameLen);

        do {
            if (answer->version < PROTOCOL_VERSION_V1_3) {
                break;
            }

            huint32 tableLen = sizeof(HUpdateDevInfoAsk) + answer->nameLen + sizeof(huint16);
            if (tableLen > data.Size()) {
                break;
            }

            const huint8 *xmlPtr = reinterpret_cast<const huint8 *>(answer->name) + answer->nameLen;
            huint16 xmlLen = (*xmlPtr << 8 | *(xmlPtr + 1));
            // -1是为了兼容名字为空时, 有设备名字字符串不存在1个空字符
            if (tableLen + xmlLen - 1 > data.Size()) {
                break;
            }
            ask.xml = std::string(reinterpret_cast<const char *>(xmlPtr) + sizeof(huint16), xmlLen);
        } while (false);

        NotifyCmd(kReadyRead, data, HReadProtocolData(HReadProtocolData::kDetectDevice, std::move(ask)));
    } break;
    default:
        break;
    }
    return true;
}
