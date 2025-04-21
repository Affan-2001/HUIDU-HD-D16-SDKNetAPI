

#include <Protocol/HSDKProtocol.h>
#include <Protocol/HStruct.h>
#include <Tool/HTool.h>
#include <Tool/tinyxml2.h>
#include <CatString.h>
#include <HCatTool.h>


#include <utility>


#define XML_MAX                     (9200)
#define LOCAL_TCP_VERSION           (0x1000007)
#define LOCAL_UDP_VERSION       	(0x1000007)

#define READ_FILE_MAX               (1024 * 1024)


using namespace cat;



HSDKProtocol::HSDKProtocol(const EventCoreWeakPtr &core)
    : IProtocolBase(core)
    , sendfaild_(false)
    , negotiate_(false)
    , processing_(false)
    , threadRun_(false)
{

}

HSDKProtocol::~HSDKProtocol()
{
    // 线程正在发送文件, 使其失败
    sendfaild_ = true;
    while (threadRun_ && !core_.expired()) {
        sendfaild_ = true;
        std::this_thread::yield();
    }
}

bool HSDKProtocol::Dispose(hint32 cmd, const HCatBuffer &data, const HCatAny &userData)
{
    switch (cmd) {
    case kInit: {
        this->InitData();
        return true;
    } break;
    case kReset: {
        this->negotiate_ = true;
        return true;
    } break;
    case kSendFaildAsk: {
        this->sendfaild_ = true;
        return true;
    } break;
    case kConnect: {
        this->sendfaild_ = false;
        return this->ParseNegotiate(0);
    } break;
    case kReadData: {
        return this->ParseReadData(data, userData);
    } break;
    default:
        break;
    }

    if (processing_) {
        ParseLog("Processing..., Data will be sent late");
        delaySend_.emplace_back(delaySend(cmd, data, userData));
        return true;
    }

    switch (cmd) {
    case kHeartBeat: {
        if (negotiate_ == false) {
            return true;
        }
        return this->ParseHeartBeat();
    } break;
    case kSendSDK: {
        return this->ParseSendSDK(cmd, data, userData);
    } break;
    case kSendFile: {
        if (negotiate_ == false) {
            delaySend_.emplace_back(delaySend(cmd, data, userData));
            return true;
        }

        processing_ = userData.IsType<HSendProtocolFileInfo>();
        if (processing_ == false) {
            ParseLog(fmt::format("Send file type error[{}]", userData.Type().name()));
            return false;
        }

        sendFileList_ = userData.Cast<HSendProtocolFileInfo>();
        if (sendFileList_.fileList.empty()) {
            processing_ = false;
            return true;
        }

        threadRun_ = !core_.expired();
        ICatEventApi::AddThread(core_, this, &HSDKProtocol::SendFileStart, sendFileList_.fileList.front(), userData);
        return true;
    } break;
    default:
        break;
    }

    return false;
}

std::list<HCatBuffer> HSDKProtocol::SplitSdkData(const HCatBuffer &xml)
{
    std::list<HCatBuffer> queue;
    int size = xml.Size();
    huint32 index = 0;

    for (; size > XML_MAX; size -= XML_MAX, index += XML_MAX){
        HSDKCmd sdk;
        sdk.len = static_cast<huint16>(sizeof(HSDKCmd)) + static_cast<huint16>(XML_MAX);
        sdk.cmd = HCmdType::kSDKCmdAsk;
        sdk.total = static_cast<huint32>(xml.Size());
        sdk.index = index;

        HCatBuffer data = HTool::iconv(sdk.len);
        data.Append(HTool::iconv(sdk.cmd));
        data.Append(HTool::iconv(sdk.total));
        data.Append(HTool::iconv(sdk.index));
        data.Append(HCatBuffer(xml.Data() + index, XML_MAX));

        queue.emplace_back(std::move(data));
    }

    if (size > 0){
        HSDKCmd sdk;
        sdk.len = static_cast<huint16>(sizeof(HSDKCmd)) + static_cast<huint16>(size);
        sdk.cmd = HCmdType::kSDKCmdAsk;
        sdk.total = static_cast<huint32>(xml.Size());
        sdk.index = index;

        HCatBuffer data = HTool::iconv(sdk.len);
        data.Append(HTool::iconv(sdk.cmd));
        data.Append(HTool::iconv(sdk.total));
        data.Append(HTool::iconv(sdk.index));
        data.Append(HCatBuffer(xml.Data() + index, size));

        queue.emplace_back(std::move(data));
    }

    return queue;
}

std::list<HCatBuffer> HSDKProtocol::SplitFileData(const HCatBuffer &fileData)
{
    std::list<HCatBuffer> queue;
    int size = fileData.Size();
    huint32 index = 0;

    for (; size > XML_MAX; size -= XML_MAX, index += XML_MAX){
        HHeader sdk;
        sdk.len = static_cast<huint16>(sizeof(HHeader)) + static_cast<huint16>(XML_MAX);
        sdk.cmd = HCmdType::kFileContentAsk;

        HCatBuffer data = HTool::iconv(sdk.len);
        data.Append(HTool::iconv(sdk.cmd));
        data.Append(HCatBuffer(fileData.ConstData() + index, XML_MAX));

        queue.emplace_back(std::move(data));
    }

    if (size > 0){
        HHeader sdk;
        sdk.len = static_cast<huint16>(sizeof(HHeader)) + static_cast<huint16>(size);
        sdk.cmd = HCmdType::kFileContentAsk;

        HCatBuffer data = HTool::iconv(sdk.len);
        data.Append(HTool::iconv(sdk.cmd));
        data.Append(HCatBuffer(fileData.ConstData() + index, size));

        queue.emplace_back(std::move(data));
    }

    return queue;
}

void HSDKProtocol::InitData()
{
    negotiate_ = false;
    processing_ = false;
    sendfaild_ = false;
    threadRun_ = false;
    sdkBuff_.Clear();
    readBuff_.Clear();
    delaySend_.clear();
    sendFileList_.Clear();
}

bool HSDKProtocol::SendData(const HCatBuffer &data, const HCatAny &userData)
{
    SendNotSDKData(data, userData);
    const HSDKCmd *sdk = reinterpret_cast<const HSDKCmd *>(data.ConstData());
    ParseLog(fmt::format("cmd-{}, Send SDK Data[{}], index[{}], total[{}]", HCmdType::HCmdTypeStr(sdk->cmd), data.Size() - sizeof(HSDKCmd), sdk->index, sdk->total));
    return !sendfaild_;
}

bool HSDKProtocol::SendNotSDKData(const HCatBuffer &data, const HCatAny &userData)
{
    const HHeader *header = reinterpret_cast<const HHeader *>(data.ConstData());
    ParseLog(fmt::format("send-{}", HCmdType::HCmdTypeStr(header->cmd)));
    return SendSourceData(data, userData);
}

bool HSDKProtocol::SendSourceData(const HCatBuffer &data, const HCatAny &userData)
{
    NotifyCmd(kSendData, data, userData);
    return !sendfaild_;
}

void HSDKProtocol::NotifyReadyData(const HCatBuffer &data, const HCatAny &userData)
{
    if (data.Empty()) {
        NotifyCmd(kReadyRead, data, userData);
        return ;
    }

    tinyxml2::XMLDocument doc;
    doc.Parse(data.ConstData(), data.Size());
    do {
        if (doc.Error()) {
            ParseLog(fmt::format("read xml error, line[{}], name[{}] str[{}]", doc.ErrorLineNum(), doc.ErrorName(), doc.ErrorStr()));
            break;
        }

        tinyxml2::XMLElement *sdk = doc.FirstChildElement("sdk");
        if (!sdk) {
            ParseLog("read xml error, sdk tag not found");
            break;
        }

        tinyxml2::XMLElement *node = sdk->FirstChildElement("out");
        if (!node) {
            ParseLog("read xml error, out tag not found");
            break;
        }

        ParseLog(fmt::format("{}-{}", node->Attribute("method"), HCmdType::HCmdTypeStr(HCmdType::kSDKCmdAnswer)));
    } while (false);


    NotifyCmd(kReadyRead, data, userData);
}

void HSDKProtocol::NotifySendFileStatus(const HCatBuffer &data, const HCatAny &userData)
{
    NotifyCmd(kUploadFileProgress, data, userData);
}

void HSDKProtocol::NotifyReset()
{
    NotifyCmd(kSendFaildAnswer, HCatBuffer(), HCatAny());
}

bool HSDKProtocol::ParseReadData(const HCatBuffer &data, const HCatAny &userData)
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
    ParseLog(fmt::format("read-{}", HCmdType::HCmdTypeStr(header.cmd)));

    bool result = true;
    switch (header.cmd) {
    case HCmdType::kTcpHeartbeatAnswer:
        break;
    case HCmdType::kSDKServiceAnswer: {
        result = this->ParseSDKServiceAnswer(buff);
    } break;
    case HCmdType::kSDKCmdAnswer: {
        result = this->ParseSDKCmdAnswer(buff);
    } break;
    case HCmdType::kErrorAnswer: {
        const HErrorStatus *status = reinterpret_cast<const HErrorStatus *>(buff.ConstData());
        processing_ = false;
        ParseLog(fmt::format("read error code[{}], status[{}]", HCmdType::HCmdTypeStr(status->head.cmd), HErrorCode::HErrorCodeStr(status->status)));
        NotifyReadyData(HCatBuffer(), HReadProtocolData(HReadProtocolData::kErrorStatus, status->status));
    } break;
    case HCmdType::kFileStartAnswer: {
        processing_ = true;
        result = this->ParseFileStartAnswer(buff);
    } break;
    case HCmdType::kFileContentAnswer:
        break;
    case HCmdType::kFileEndAnswer: {
        std::string filePath;
        if (sendFileList_.fileList.empty() == false) {
            filePath = sendFileList_.fileList.front().filePath;
            sendFileList_.fileList.pop_front();
        }

        hint64 fileSize = GetFileSize(filePath);
        this->NotifySendFileStatus(fmt::format("{};;{};;{};;2", filePath, fileSize, fileSize));

        if (sendFileList_.fileList.empty() == false) {
            threadRun_ = true;
            ICatEventApi::AddThread(core_, this, &HSDKProtocol::SendFileStart, sendFileList_.fileList.front(), userData);
            break;
        }

        processing_ = false;
        result = this->DelaySend();
    } break;
    default:
        ParseLog(fmt::format("Unresolved-{}", HCmdType::HCmdTypeStr(header.cmd)));
        break;
    }

    if (result && readBuff_.Empty() == false) {
        return this->ParseReadData(HCatBuffer(), userData);
    }

    return result;
}

bool HSDKProtocol::ParseNegotiate(int num)
{
    HCatBuffer data;
    if (num == 0) {
        HTcpVersion version;
        version.len = sizeof(HTcpVersion);
        version.cmd = HCmdType::kSDKServiceAsk;
        version.version = LOCAL_TCP_VERSION;

        data = HTool::iconv(version.len);
        data.Append(HTool::iconv(version.cmd));
        data.Append(HTool::iconv(version.version));
    } else {
        tinyxml2::XMLDocument doc;
        tinyxml2::XMLDeclaration* declaration = doc.NewDeclaration();
        doc.InsertFirstChild(declaration);
        tinyxml2::XMLElement* root = doc.NewElement("sdk");
        root->SetAttribute("guid", "##GUID");
        doc.InsertEndChild(root);

        tinyxml2::XMLElement* inNode = doc.NewElement("in");
        inNode->SetAttribute("method", "GetIFVersion");
        root->InsertEndChild(inNode);

        tinyxml2::XMLElement* node = doc.NewElement("version");
        node->SetAttribute("value", "1000000");
        inNode->InsertEndChild(node);

        tinyxml2::XMLPrinter xmlStr;
        doc.Print(&xmlStr);
        HCatBuffer request(xmlStr.CStr());

        HSDKCmd sdk;
        sdk.len = static_cast<huint16>(sizeof(HSDKCmd)) + static_cast<huint16>(request.Size());
        sdk.cmd = HCmdType::kSDKCmdAsk;
        sdk.total = static_cast<huint32>(request.Size());
        sdk.index = 0;
        data = HTool::iconv(sdk.len);
        data.Append(HTool::iconv(sdk.cmd));
        data.Append(HTool::iconv(sdk.total));
        data.Append(HTool::iconv(sdk.index));
        data.Append(std::move(request));
    }

    if (SendNotSDKData(data) == false) {
        this->NotifyReset();
        ParseLog("Negotiate faild.");
        return false;
    }

    return true;
}

bool HSDKProtocol::ParseSDKServiceAnswer(const HCatBuffer &data)
{
    const HTcpVersion *version = reinterpret_cast<const HTcpVersion *>(data.ConstData());
    if (version->version > LOCAL_TCP_VERSION) {
        ParseLog("Version Toolow.");
        return false;
    }
    return ParseNegotiate(1);
}

bool HSDKProtocol::ParseSDKCmdAnswer(HCatBuffer &data)
{
    HSDKCmd sdkCmd(std::string(data.ConstData(), sizeof(HSDKCmd)));
    data.Remove(0, sizeof(HSDKCmd));

    // 检查协商最后获取guid
    if (negotiate_ == false) {
        tinyxml2::XMLDocument doc;
        tinyxml2::XMLError code = doc.Parse(data.ConstData(), data.Size());
        if (code != tinyxml2::XML_SUCCESS) {
            return false;
        }

        tinyxml2::XMLElement *node = doc.FirstChildElement("sdk");
        guid_ = node->Attribute("guid");
        NotifyCmd(kNotofyConnect, HCatBuffer(), HCatAny());

        // 完成协商, 开始进行延迟发送
        negotiate_ = true;
        return DelaySend();
    }

    if (data.Empty() == false) {
        sdkBuff_.Append(data);
        if (sdkCmd.total <= sdkBuff_.Size()) {
            this->NotifyReadyData(sdkBuff_);
            sdkBuff_.Clear();
        }
    }

    return true;
}

bool HSDKProtocol::ParseHeartBeat()
{
    HHeader heartbeat(sizeof(HHeader), HCmdType::kTcpHeartbeatAsk);
    HCatBuffer data;
    data.Append(HTool::iconv(heartbeat.len));
    data.Append(HTool::iconv(heartbeat.cmd));
    return SendNotSDKData(data);
}


bool HSDKProtocol::ParseSendSDK(hint32 cmd, const HCatBuffer &data, const HCatAny &userData)
{
    // 还未协商完, 完成后会自动补发
    if (negotiate_ == false) {
        delaySend_.emplace_back(delaySend(cmd, data, userData));
        return true;
    }

    if (data.Empty()) {
        return true;
    }

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError code = doc.Parse(data.ConstData(), data.Size());
    HCatBuffer sdkData(data);
    if (code != tinyxml2::XML_SUCCESS) {
        std::string guid("##GUID");
        sdkData.Replace(guid, guid_);
    } else {
        tinyxml2::XMLElement *node = doc.FirstChildElement("sdk");
        node->SetAttribute("guid", guid_.data());

        tinyxml2::XMLPrinter xmlStr;
        doc.Print(&xmlStr);
        sdkData = xmlStr.CStr();
    }

    auto queue = SplitSdkData(sdkData);
    for (auto &i : queue) {
        if (SendData(i, userData) == false) {
            this->NotifyReset();
            return false;
        }
    }

    return true;
}

bool HSDKProtocol::SendFileStart(const HSendFile &sendFile, const HCatAny &userData)
{
    RallPoint<bool> threadRun(&threadRun_, false);
    RallPoint<bool> sending(&processing_, false);
    if (sendfaild_) {
        return false;
    }

    std::string filePath = sendFile.filePath;
    std::string md5 = sendFile.md5;
    if (md5.empty()) {
        md5 = HTool::MD5_U8(filePath, &sendfaild_);
    }

    if (sendfaild_) {
        return false;
    }

    if (md5.empty()) {
        ParseLog(fmt::format("File cannot be opened. file[{}]", filePath));
        return false;
    }

    std::string fileName = filePath.substr(filePath.find_last_of("/\\") + 1);
    HCatBuffer fileAsk(fileName.size() + sizeof(HFileStartAsk) - sizeof(hint8), '\0');

    HFileStartAsk *ask = reinterpret_cast<HFileStartAsk *>(fileAsk.Data());
    ask->len = static_cast<huint16>(fileAsk.Size());
    ask->cmd = HCmdType::kFileStartAsk;
    ask->size = GetFileSize(filePath);
    strcpy(ask->md5, md5.data());
    strcpy(ask->name, fileName.data());
    ask->type = sendFile.type;

    if (SendNotSDKData(fileAsk, userData) == false) {
        this->NotifyReset();
        return false;
    }

    sending.Take();
    return true;
}


bool HSDKProtocol::ParseFileStartAnswer(const HCatBuffer &data)
{
    RallPoint<bool> sending(&processing_, false);

    do {
        if (sendFileList_.fileList.empty()) {
            ParseLog("Sending file name is empty.");
            return false;
        }

        const HFileStartAnswer *answer = reinterpret_cast<const HFileStartAnswer *>(data.ConstData());
        if (answer->status != 0) {
            ParseLog(fmt::format("read status error[{}]", HErrorCode::HErrorCodeStr(answer->status)));
            NotifyReadyData(HCatBuffer(), HReadProtocolData(HReadProtocolData::kErrorStatus, answer->status));
            return false;
        }

        std::string filePath = sendFileList_.fileList.front().filePath;
        threadSendFilePtr callBack(new FileCallBack);
        callBack->sendFile.open(IconvFilePath(filePath), std::ios::in | std::ios::binary);
        if (callBack->sendFile.is_open() == false) {
            ParseLog(fmt::format("open file[{}] faild", filePath));
            break;
        }

        callBack->sendFile.seekg(0, std::ios::end);
        hint64 fileSize = callBack->sendFile.tellg();
        callBack->sendFile.seekg(0, std::ios::beg);
        hint64 sendSize = answer->existSize;

        ParseLog(fmt::format("File exists size[{}]", sendSize));
        if (fileSize == sendSize) {
            ParseLog(fmt::format("File already exists. file[{}]", filePath));
            NotifySendFileStatus(fmt::format("{};;{};;{};;0", filePath, sendSize, fileSize));
            break;
        }

        callBack->GetSendData(sendSize);
        callBack->filePath = filePath;
        callBack->fileSize = fileSize;
        callBack->sendSize = sendSize;
        if (callBack->sendData.Empty()) {
            break;
        }

        if (SendNotSDKData(callBack->sendData) == false) {
            ParseLog(fmt::format("File sending failed. file[{}]. send size[{}]", filePath, sendSize));
            return false;
        }

        callBack->sendSize += callBack->readSize;
        NotifySendFileStatus(fmt::format("{};;{};;{};;0", callBack->filePath, callBack->sendSize, fileSize));
        ParseLog(fmt::format("[{}] sending size[{}]/[{}]", callBack->filePath, callBack->sendSize, fileSize));
        sending.Take();
        threadRun_ = !core_.expired();
        ICatEventApi::AddThread(core_, this, &HSDKProtocol::ThreadSendFile, HCatAny(callBack));
        return true;

    } while (false);

    return this->SendFileEnd();
}

bool HSDKProtocol::ThreadSendFile(const HCatAny &userData)
{
    RallPoint<bool> threadRun(&threadRun_, false);
    if (sendfaild_) {
        return false;
    }

    if (userData.IsType<threadSendFilePtr>() == false) {
        SendFileEnd();
        return false;
    }

    threadSendFilePtr callBack = userData.Cast<threadSendFilePtr>();
    for (;;) {
        if (sendfaild_) {
            return false;
        }

        callBack->GetSendData();
        if (callBack->sendData.Empty()) {
            break;
        }

        if (SendNotSDKData(callBack->sendData) == false) {
            ICatEventApi::Forward(core_, this, &HSDKProtocol::NotifySendFileStatus, fmt::format("{};;{};;{};;1", callBack->filePath, callBack->sendSize, callBack->fileSize), userData);
            return false;
        }

        callBack->sendSize += callBack->readSize;

        ICatEventApi::Forward(core_, this, &HSDKProtocol::NotifySendFileStatus, fmt::format("{};;{};;{};;0", callBack->filePath, callBack->sendSize, callBack->fileSize), userData);
    }

    return SendFileEnd();
}

bool HSDKProtocol::SendFileEnd()
{
    HCatBuffer sendEnd(sizeof(HHeader));
    HHeader *ask = reinterpret_cast<HHeader *>(sendEnd.Data());
    ask->len = sizeof(HHeader);
    ask->cmd = HCmdType::kFileEndAsk;
    if (SendNotSDKData(sendEnd) == false) {
        this->NotifyReset();
        ParseLog("Send file End request faild");
        return false;
    }
    return true;
}

bool HSDKProtocol::DelaySend()
{
    std::list<delaySend> sendList;
    sendList.swap(delaySend_);
    while (sendList.empty() == false) {
        if (processing_) {
            delaySend_.splice(delaySend_.begin(), std::move(sendList));
            break;
        }

        delaySend i = std::move(sendList.front());
        sendList.pop_front();

        if (Dispose(i.cmd, i.data, i.userData) == false) {
            ParseLog("Delay Send Faild.");
            delaySend_.clear();
            return false;
        }
    }

    return true;
}

hint64 HSDKProtocol::GetFileSize(const std::string &filePath)
{
    std::fstream sendFile;
    sendFile.open(IconvFilePath(filePath), std::ios::in | std::ios::binary);
    if (sendFile.is_open() == false) {
        ParseLog(fmt::format("open file[{}] faild", filePath));
        return 0;
    }

    sendFile.seekg(0, std::ios::end);
    hint64 fileSize = sendFile.tellg();
    sendFile.close();
    return fileSize;
}

std::string HSDKProtocol::IconvFilePath(const std::string &filePath) const
{
#ifdef H_WIN
    return cat::HCatTool::UTF8ToANSI(filePath);
#else
    return filePath;
#endif
}

void HSDKProtocol::ParseLog(const std::string &log)
{
#ifdef DEBUG_LOG_SIGNAL
    ICatEventApi::ForwardSignal(core_, this, DebugLog, log);
#endif
}

bool HSDKProtocol::FileCallBack::GetSendData(hint64 offset)
{
    sendData.Clear();

    if (sendFile.is_open() == false) {
        return false;
    }

    if (offset != -1) {
        sendFile.seekg(offset);
    }

    std::vector<char> data(READ_FILE_MAX);
    sendFile.read(data.data(), data.size());
    readSize = sendFile.gcount();
    if (readSize <= 0) {
        sendFile.close();
        return false;
    }

    auto splitData = SplitFileData(HCatBuffer(data.data(), static_cast<std::size_t>(readSize)));
    for (auto &&i: splitData) {
        sendData.Append(std::move(i));
    }
    return true;
}
