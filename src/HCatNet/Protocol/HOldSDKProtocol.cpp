

#include <Protocol/HOldSDKProtocol.h>
#include <Protocol/HStruct.h>
#include <Tool/HTool.h>
#include <CatString.h>


#define XML_MAX                     (9200)
#define LOCAL_TCP_VERSION           (0x1000007)
#define LOCAL_UDP_VERSION       	(0x1000007)


using namespace cat;



HOldSDKProtocol::HOldSDKProtocol(const EventCoreWeakPtr &core)
    : IProtocolBase(core)
    , negotiate_(false)
    , sendfaild_(false)
{

}

HOldSDKProtocol::~HOldSDKProtocol()
{

}

bool HOldSDKProtocol::Dispose(hint32 cmd, const HCatBuffer &data, const HCatAny &userData)
{
    switch (cmd) {
    case kInit: {
        this->InitData();
        return true;
    } break;
    case kReset: {
        negotiate_ = true;
        return true;
    } break;
    case kSendFaildAsk: {
        this->sendfaild_ = true;
        return true;
    } break;
    case kConnect: {
        this->sendfaild_ = false;
        this->ParseNegotiate(0);
    } break;
    case kReadData: {
        return this->ParseReadData(data, userData);
    } break;
    case kHeartBeat: {
        return this->ParseHeartBeat();
    } break;
    case kSendSDK: {
        return this->ParseSendSDK(cmd, data, userData);
    } break;
    case kSendFile: {
        ParseLog("Send file not realized");
        return true;
    } break;
    default:
        break;
    }

    return false;
}

std::list<HOldSDKProtocol::SDKData> HOldSDKProtocol::SplitSdkData(const HCatBuffer &xml)
{
    std::list<HOldSDKProtocol::SDKData> queue;
    int size = xml.Size();
    huint32 index = 0;

    do {
        SDKData sdkData;
        HContentStartAsk ask;
        sdkData.cmd = ask.pCmd;
        HCatBuffer data = HTool::iconv(ask.pLen);
        data.Append(HTool::iconv(ask.pCmd));
        data.Append(HTool::iconv(ask.inCmd));
        data.Append(HTool::iconv(ask.inLen));
        sdkData.sendData.emplace_back(std::move(data));
        queue.emplace_back(std::move(sdkData));
    } while (false);

    for (; size > XML_MAX; size -= XML_MAX, index += XML_MAX){
        SDKData sdkData;
        HContentDataAsk ask;
        sdkData.cmd = ask.cmd;
        ask.len = XML_MAX + ask.len;
        HCatBuffer data = HTool::iconv(ask.len);
        data.Append(HTool::iconv(ask.cmd));
        data.Append(HTool::iconv(ask.inCmd));
        data.Append(HCatBuffer(xml.ConstData() + index, XML_MAX));
        sdkData.sendData.emplace_back(std::move(data));
        queue.emplace_back(std::move(sdkData));
    }

    if (size > 0){
        SDKData sdkData;
        HContentDataAsk ask;
        sdkData.cmd = ask.cmd;
        ask.len = size + ask.len;
        HCatBuffer data;
        data = HTool::iconv(ask.len);
        data.Append(HTool::iconv(ask.cmd));
        data.Append(HTool::iconv(ask.inCmd));
        data.Append(HCatBuffer(xml.ConstData() + index, size));
        sdkData.sendData.emplace_back(std::move(data));
        queue.emplace_back(std::move(sdkData));
    }

    do {
        SDKData sdkData;
        HContentEndAsk ask;
        sdkData.cmd = ask.cmd;
        HCatBuffer data;
        data = HTool::iconv(ask.len);
        data.Append(HTool::iconv(ask.cmd));
        data.Append(HTool::iconv(ask.inCmd));
        sdkData.sendData.emplace_back(std::move(data));
        queue.emplace_back(std::move(sdkData));
    } while (false);

    return queue;
}

bool HOldSDKProtocol::SendData(const HCatBuffer &data, const HCatAny &userData)
{
    SendNotSDKData(data, userData);
    const HContentStartAnswer *sdk = reinterpret_cast<const HContentStartAnswer *>(data.ConstData());
    ParseLog(fmt::format("cmd-{}, Send Old SDK Data[{}], len[{}], inCmd[{}]", HOldCmdType::HOldCmdTypeStr(sdk->cmd), data.Size() - sizeof(HSDKCmd), sdk->len, sdk->inCmd));
    return !sendfaild_;
}

bool HOldSDKProtocol::SendNotSDKData(const HCatBuffer &data, const HCatAny &userData)
{
    const HHeader *header = reinterpret_cast<const HHeader *>(data.ConstData());
    ParseLog(fmt::format("send-{}", HOldCmdType::HOldCmdTypeStr(header->cmd)));
    return SendSourceData(data, userData);
}

bool HOldSDKProtocol::SendSourceData(const HCatBuffer &data, const HCatAny &userData)
{
    NotifyCmd(kSendData, data, userData);
    return !sendfaild_;
}


void HOldSDKProtocol::InitData()
{
    delaySend_.clear();
    sendQueue_.clear();
    buff_.Clear();
    readBuff_.Clear();
    negotiate_ = false;
}

bool HOldSDKProtocol::ParseNegotiate(int num)
{
    HCatBuffer data;
    if (num == 0) {
        HTcpVersion version;
        version.len = sizeof(HTcpVersion);
        version.cmd = static_cast<huint16>(HOldCmdType::kVersionAsk);
        version.version = LOCAL_TCP_VERSION;

        data = HTool::iconv(version.len);
        data.Append(HTool::iconv(version.cmd));
        data.Append(HTool::iconv(version.version));
    } else if (num == 1) {
        HHeader sdk;
        sdk.cmd = static_cast<huint16>(HOldCmdType::kTcpTranInAsk);
        data = HTool::iconv(sdk.len);
        data.Append(HTool::iconv(sdk.cmd));
    }

    if (SendNotSDKData(data) == false) {
        ParseLog("Negotiate faild.");
        NotifyCmd(kSendFaildAnswer, HCatBuffer(), HCatAny());
        return false;
    }

    return true;
}

bool HOldSDKProtocol::ParseHeartBeat()
{
    HHeader answer;
    answer.cmd = static_cast<huint16>(HOldCmdType::kTcpHeartbeatPacketAsk);
    HCatBuffer sendData(reinterpret_cast<char *>(&answer), answer.len);
    return SendNotSDKData(sendData);
}

bool HOldSDKProtocol::ParseSendSDK(hint32 cmd, const HCatBuffer &data, const HCatAny &userData)
{
    // 还未协商完, 完成后会自动补发
    if (negotiate_ == false) {
        delaySend_.emplace_back(delaySend(cmd, data, userData));
        return true;
    }

    auto sendData = SplitSdkData(data);
    if (sendData.empty()) {
        return false;
    }

    // 还在发送状态, 加入发送队列
    if (sendQueue_.empty() == false) {
        sendQueue_.splice(sendQueue_.end(), std::move(sendData));
        return true;
    }

    HCatBuffer sdkData = std::move(sendData.front().sendData.front());
    sendData.front().sendData.pop_front();
    if (SendNotSDKData(sdkData, userData) == false) {
        NotifyCmd(kSendFaildAnswer, HCatBuffer(), HCatAny());
        return false;
    }

    if (sendData.front().sendData.empty()) {
        sendData.pop_front();
    }

    if (sendData.empty() == false) {
        sendQueue_.splice(sendQueue_.end(), std::move(sendData));
    }

    return true;
}

bool HOldSDKProtocol::ParseReadData(const HCatBuffer &data, const HCatAny &userData)
{
    readBuff_.Append(data);
    HHeader header(readBuff_.GetString(sizeof(HHeader)));

    if (readBuff_.Size() < sizeof(HHeader)) {
        return true;
    }

    if (header.len > readBuff_.Size()) {
        return true;
    }

    // 切包
    HCatBuffer buff = readBuff_.Mid(0, header.len);
    readBuff_.Remove(0, header.len);

    // 输出日志信息
    ParseLog(fmt::format("read-{}", HOldCmdType::HOldCmdTypeStr(header.cmd)));

    switch (header.cmd) {
    case HOldCmdType::kVersionAnswer: {
        const HTcpVersion *version = reinterpret_cast<const HTcpVersion *>(buff.ConstData());
        if (version->version > LOCAL_TCP_VERSION) {
            ParseLog("Version Toolow.");
            return false;
        }
        return ParseNegotiate(1);
    } break;
    case HOldCmdType::kTcpTranInAnswer: {
        const HTcpVersion *version = reinterpret_cast<const HTcpVersion *>(buff.ConstData());
        if (version->version) {
            ParseLog("The device is occupied!");
            return false;
        }
        negotiate_ = true;
        HHeader answer;
        answer.cmd = static_cast<huint16>(HOldCmdType::kTestDeviceLockerAsk);
        HCatBuffer sendData(reinterpret_cast<char *>(&answer), answer.len);
        return SendNotSDKData(sendData);
    } break;
    case HOldCmdType::kTestDeviceLockerAnswer: {
        const HTestDeviceLockerAnswer *answer = reinterpret_cast<const HTestDeviceLockerAnswer *>(data.ConstData());
        if (answer->enable) {
            ParseLog("The Device is Occupied");
            return false;
        }

        NotifyCmd(kNotofyConnect, HCatBuffer(), HCatAny());
        while (delaySend_.empty() == false) {
            delaySend sendData = std::move(delaySend_.front());
            delaySend_.pop_front();
            if (Dispose(sendData.cmd, sendData.data, sendData.userData) == false) {
                delaySend_.clear();
                return false;
            }
        }
        return true;
    } break;
    case HOldCmdType::kContentStartAsk: {
        HContentStartAnswer answer;
        HCatBuffer sendData(reinterpret_cast<char *>(&answer), answer.len);
        return SendNotSDKData(sendData, userData);
    } break;
    case HOldCmdType::kContentStartAnswer: {
        if (sendQueue_.empty()) {
            return false;
        }

        HCatBuffer data = std::move(sendQueue_.front().sendData.front());
        sendQueue_.front().sendData.pop_front();
        if (sendQueue_.front().sendData.empty()) {
            sendQueue_.pop_front();
        }

        return SendNotSDKData(data, userData);
    } break;
    case HOldCmdType::kContentDataAsk: {
        const HContentDataAsk *ask = reinterpret_cast<const HContentDataAsk *>(buff.ConstData());
        int len = ask->len - sizeof(HContentDataAnswer);
        buff_.Append(HCatBuffer(reinterpret_cast<const char *>(ask->data), len));
        HContentDataAnswer answer;
        HCatBuffer sendData(reinterpret_cast<char *>(&answer), answer.len);
        if (SendNotSDKData(sendData, userData) == false) {
            buff_.Clear();
            NotifyCmd(kSendFaildAnswer, HCatBuffer(), HCatAny());
            return false;
        }
        return true;
    } break;
    case HOldCmdType::kContentDataAnswer:
    case HOldCmdType::kContentEndAnswer: {
        if (sendQueue_.empty()) {
            return true;
        }

        HCatBuffer data = std::move(sendQueue_.front().sendData.front());
        sendQueue_.front().sendData.pop_front();;
        if (sendQueue_.front().sendData.empty()) {
            sendQueue_.pop_front();
        }

        if (SendNotSDKData(data, userData) == false) {
            sendQueue_.clear();
            NotifyCmd(kSendFaildAnswer, HCatBuffer(), HCatAny());
            return false;
        }

        return true;
    } break;
    case HOldCmdType::kContentEndAsk: {
        HContentEndAnswer answer;
        HCatBuffer sendData(reinterpret_cast<char *>(&answer), answer.len);
        bool ret = SendNotSDKData(sendData, userData);

        if (buff_.Empty() == false) {
            NotifyCmd(kReadyRead, buff_, userData);
            buff_.Clear();
        }

        return ret;
    }
    case HOldCmdType::kTcpHeartbeatPacketAnswer: {
        return this->ParseHeartBeat();
    } break;
    default: {
        ParseLog(fmt::format("Untreated-{}", header.cmd));
    } break;
    }

    return false;
}

void HOldSDKProtocol::ParseLog(const std::string &log)
{
#ifdef DEBUG_LOG_SIGNAL
    ICatEventApi::ForwardSignal(core_, this, DebugLog, log);
#endif
}
