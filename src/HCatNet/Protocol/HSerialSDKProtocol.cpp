

#include <Protocol/HSerialSDKProtocol.h>
#include <Protocol/HStruct.h>
#include <Tool/HTool.h>
#include <Tool/tinyxml2.h>
#include <CatString.h>


#define XML_MAX     (9000)


using namespace cat;



HSerialSDKProtocol::HSerialSDKProtocol(const EventCoreWeakPtr &core)
    : IProtocolBase(core)
{

}

HSerialSDKProtocol::~HSerialSDKProtocol()
{

}

bool HSerialSDKProtocol::Dispose(hint32 cmd, const HCatBuffer &data, const HCatAny &userData)
{
    switch (cmd) {
    case kReset:
    case kHeartBeat: {
        return true;
    } break;
    case kInit: {
        this->InitData();
        return true;
    } break;
    case kSendFaildAsk: {
        this->sendfaild_ = true;
        return true;
    } break;
    case kConnect: {
        this->sendfaild_ = false;
        NotifyCmd(kNotofyConnect, data, userData);
        return true;
    } break;
    case kReadData: {
        return this->ParseReadData(data, userData);
    } break;
    case kSendSDK: {
        return this->ParseSendSDK(data, userData);
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

std::list<HCatBuffer> HSerialSDKProtocol::SplitSdkData(const HCatBuffer &xml)
{
    std::list<HCatBuffer> queue;
    int size = xml.Size();
    huint32 index = 0;

    for (; size > XML_MAX; size -= XML_MAX, index += XML_MAX){
        HCatBuffer xmlData(xml.ConstData() + index, XML_MAX);
        HSerialSDKCmd sdk;
        sdk.len = static_cast<huint16>(sizeof(HSerialSDKCmd)) + static_cast<huint16>(XML_MAX);
        sdk.cmd = HCmdType::kSDKCmdAsk;
        sdk.total = static_cast<huint32>(xml.Size());
        sdk.index = index;
        sdk.crc32 = HTool::crc32(xmlData.ConstData(), xmlData.Size());

        HCatBuffer data;
        data = HTool::iconv(sdk.len);
        data.Append(HTool::iconv(sdk.cmd));
        data.Append(HTool::iconv(sdk.total));
        data.Append(HTool::iconv(sdk.index));
        data.Append(HTool::iconv(sdk.crc32));
        data.Append(std::move(xmlData));

        queue.emplace_back(std::move(data));
    }

    if (size > 0){
        HCatBuffer xmlData(xml.ConstData() + index, size);
        HSerialSDKCmd sdk;
        sdk.len = static_cast<huint16>(sizeof(HSerialSDKCmd)) + static_cast<huint16>(size);
        sdk.cmd = HCmdType::kSDKCmdAsk;
        sdk.total = static_cast<huint32>(xml.Size());
        sdk.index = index;
        sdk.crc32 = HTool::crc32(xmlData.ConstData(), xmlData.Size());

        HCatBuffer data;
        data = HTool::iconv(sdk.len);
        data.Append(HTool::iconv(sdk.cmd));
        data.Append(HTool::iconv(sdk.total));
        data.Append(HTool::iconv(sdk.index));
        data.Append(HTool::iconv(sdk.crc32));
        data.Append(std::move(xmlData));

        queue.emplace_back(std::move(data));
    }

    return queue;
}

void HSerialSDKProtocol::InitData()
{
    readBuff_.Clear();
    sdkBuff_.Clear();
    sendfaild_ = false;
}

bool HSerialSDKProtocol::SendData(const HCatBuffer &data, const HCatAny &userData)
{
    SendNotSDKData(data, userData);
    const HSerialSDKCmd *sdk = reinterpret_cast<const HSerialSDKCmd *>(data.ConstData());
    ParseLog(fmt::format("cmd-{}, Send SDK Data[{}], index[{}], total[{}], crc32[{}]", HCmdType::HCmdTypeStr(sdk->cmd), data.Size() - sizeof(HSDKCmd), sdk->index, sdk->total, sdk->crc32));
    return !sendfaild_;
}

bool HSerialSDKProtocol::SendNotSDKData(const HCatBuffer &data, const HCatAny &userData)
{
    const HHeader *header = reinterpret_cast<const HHeader *>(data.ConstData());
    ParseLog(fmt::format("send-{}", HCmdType::HCmdTypeStr(header->cmd)));
    return SendSourceData(data, userData);
}

bool HSerialSDKProtocol::SendSourceData(const HCatBuffer &data, const HCatAny &userData)
{
    NotifyCmd(kSendData, data, userData);
    return !sendfaild_;
}

bool HSerialSDKProtocol::ParseReadData(const HCatBuffer &data, const HCatAny &userData)
{
    (void)userData;
    readBuff_.Append(data);
    HHeader header(readBuff_.GetString(sizeof(HHeader)));

    if (readBuff_.Size() < sizeof(HHeader)) {
        return true;
    }

    switch (header.cmd) {
    case HCmdType::kErrorAnswer:
    case HCmdType::kSDKCmdAnswer: {
    } break;
    default: {
        this->InitData();
        ParseLog(fmt::format("Not parse cmd[{}]", HCmdType::HCmdTypeStr(header.cmd)));
    } break;
    }

    if (header.len > readBuff_.Size()) {
        return true;
    }

    // 切包
    HCatBuffer buff = readBuff_.Mid(0, header.len);
    readBuff_.Remove(0, header.len);

    // 输出日志信息
    ParseLog(fmt::format("read-{}", HCmdType::HCmdTypeStr(header.cmd)));

    switch (header.cmd) {
    case HCmdType::kSDKCmdAnswer: {
        return ParseSDKCmdAnswer(buff);
    } break;
    case HCmdType::kErrorAnswer: {
        const HErrorStatus *status = reinterpret_cast<const HErrorStatus *>(buff.ConstData());
        ParseLog(fmt::format("read error code[{}], status[{}]", HCmdType::HCmdTypeStr(status->head.cmd),  HErrorCode::HErrorCodeStr(status->status)));
        return ParseReadData(HCatBuffer(), *status);
    } break;
    default:
        break;
    }

    return false;
}

bool HSerialSDKProtocol::ParseSendSDK(const HCatBuffer &data, const HCatAny &userData)
{
    HCatBuffer sdkData(data);

    do {
        tinyxml2::XMLDocument doc;
        doc.Parse(data.ConstData(), data.Size());
        if (doc.Error()) {
            break;
        }

        tinyxml2::XMLElement *node = doc.FirstChildElement("sdk");
        node->SetAttribute("guid", "##GUID");

        tinyxml2::XMLPrinter xmlStr;
        doc.Print(&xmlStr);
        sdkData = xmlStr.CStr();
    } while (false);

    huint16 sdkCmd = 0;
    auto queue = SplitSdkData(sdkData);
    if (queue.empty() == false) {
        const HSDKCmd *sdk = reinterpret_cast<const HSDKCmd *>(queue.front().ConstData());
        sdkCmd = sdk->cmd;
    }

    for (auto &i : queue) {
        if (SendNotSDKData(i, userData) == false) {
            return false;
        }

        const HSerialSDKCmd *sdk = reinterpret_cast<const HSerialSDKCmd *>(i.ConstData());
        ParseLog(fmt::format("cmd[{}], Send Serial SDK Data[{}], index[{}], total[{}], crc32[{}]", HCmdType::HCmdTypeStr(sdkCmd), i.Size() - sizeof(HSerialSDKCmd), sdk->index, sdk->total, sdk->crc32));
    }

    return true;
}

bool HSerialSDKProtocol::ParseSDKCmdAnswer(HCatBuffer &data)
{
    HSerialSDKCmd sdkCmd(data.GetString(sizeof(HSerialSDKCmd)));
    data.Remove(0, sizeof(HSerialSDKCmd));

    if (data.Empty()) {
        return true;
    }

    sdkBuff_.Append(data);
    huint32 crc = HTool::crc32(data.ConstData(), data.Size());
    ParseLog(fmt::format("data crc32[{}] package crc32[{}]", crc, sdkCmd.crc32));

    // 数据不足
    if (sdkCmd.total > sdkBuff_.Size()) {
        return true;
    }

    // 输出日志
    do {
        tinyxml2::XMLDocument doc;
        doc.Parse(sdkBuff_.ConstData(), sdkBuff_.Size());
        if (doc.Error()) {
            break;
        }

        tinyxml2::XMLElement *sdk = doc.FirstChildElement("sdk");
        if (!sdk) {
            break;
        }

        tinyxml2::XMLElement *node = sdk->FirstChildElement("out");
        if (!node) {
            break;
        }

        ParseLog(fmt::format("{}-{}", node->Attribute("method"), HCmdType::HCmdTypeStr(HCmdType::kSDKCmdAnswer)));
    } while(false);

    NotifyCmd(kReadyRead, sdkBuff_, HCatAny());
    sdkBuff_.Clear();
    return true;
}

void HSerialSDKProtocol::ParseLog(const std::string &log)
{
#ifdef DEBUG_LOG_SIGNAL
    ICatEventApi::ForwardSignal(core_, this, DebugLog, log);
#endif
}

