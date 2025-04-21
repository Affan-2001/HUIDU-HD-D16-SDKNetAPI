

#include <Protocol/HLcdSdkProtocol.h>
#include <Protocol/HStruct.h>
#include <Tool/HTool.h>
#include <CatString.h>
#include <HCatTool.h>


#define JSON_MAX                    (9200)
#define LCD_VERSION                 (0x01000000)

#define READ_FILE_MAX               (1024 * 1024)


using namespace cat;



HLcdSdkProtocol::HLcdSdkProtocol(const EventCoreWeakPtr &core)
    : IProtocolBase(core)
    , sendfaild_(false)
    , negotiate_(false)
    , processing_(false)
    , threadRun_(false)
{

}

HLcdSdkProtocol::~HLcdSdkProtocol()
{
    // 线程正在发送文件, 使其失败
    sendfaild_ = true;
    while (threadRun_ && !core_.expired()) {
        sendfaild_ = true;
        std::this_thread::yield();
    }
}

bool HLcdSdkProtocol::Dispose(hint32 cmd, const HCatBuffer &data, const HCatAny &userData)
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
        return this->ParseNegotiate();
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

        processing_ = SendFileListStart(data, userData);
        return processing_;
    } break;
    default:
        break;
    }

    return false;
}

std::list<HCatBuffer> HLcdSdkProtocol::SplitSdkData(const HCatBuffer &json)
{
    std::list<HCatBuffer> queue;
    std::size_t size = json.Size();
    huint32 index = 0;

    for (; size > JSON_MAX; size -= JSON_MAX, index += JSON_MAX){
        HSDKCmd sdk;
        sdk.len = static_cast<huint16>(sizeof(HSDKCmd)) + static_cast<huint16>(JSON_MAX);
        sdk.cmd = HCmdType::kLCDMsgAsk;
        sdk.total = json.Size();
        sdk.index = index;

        HCatBuffer data = HTool::iconv(sdk.len);
        data.Append(HTool::iconv(sdk.cmd));
        data.Append(HTool::iconv(sdk.total));
        data.Append(HTool::iconv(sdk.index));
        data.Append(HCatBuffer(json.ConstData() + index, JSON_MAX));

        queue.emplace_back(std::move(data));
    }

    if (size > 0){
        HSDKCmd sdk;
        sdk.len = static_cast<huint16>(sizeof(HSDKCmd)) + static_cast<huint16>(size);
        sdk.cmd = HCmdType::kLCDMsgAsk;
        sdk.total = json.Size();
        sdk.index = index;

        HCatBuffer data = HTool::iconv(sdk.len);
        data.Append(HTool::iconv(sdk.cmd));
        data.Append(HTool::iconv(sdk.total));
        data.Append(HTool::iconv(sdk.index));
        data.Append(HCatBuffer(json.ConstData() + index, size));

        queue.emplace_back(std::move(data));
    }

    return queue;
}

std::list<HCatBuffer> HLcdSdkProtocol::SplitFileData(const HCatBuffer &fileData)
{
    std::list<HCatBuffer> queue;
    std::size_t size = fileData.Size();
    huint32 index = 0;

    for (; size > JSON_MAX; size -= JSON_MAX, index += JSON_MAX){
        HHeader sdk;
        sdk.len = static_cast<huint16>(sizeof(HHeader)) + static_cast<huint16>(JSON_MAX);
        sdk.cmd = HCmdType::kFileContentAsk;

        HCatBuffer data = HTool::iconv(sdk.len);
        data.Append(HTool::iconv(sdk.cmd));
        data.Append(HCatBuffer(fileData.ConstData() + index, JSON_MAX));

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

void HLcdSdkProtocol::InitData()
{
    negotiate_ = false;
    processing_ = false;
    sendfaild_ = false;
    threadRun_ = false;
    sdkBuff_.Clear();
    readBuff_.Clear();
    delaySend_.clear();
    lastFile_.clear();
}

bool HLcdSdkProtocol::SendData(const HCatBuffer &data, const HCatAny &userData)
{
    SendNotSDKData(data, userData);
    const HSDKCmd *sdk = reinterpret_cast<const HSDKCmd *>(data.ConstData());
    ParseLog(fmt::format("cmd-{}, Send LCD Data[{}], offset[{}], total[{}]", HCmdType::HCmdTypeStr(sdk->cmd), data.Size() - sizeof(HSDKCmd), sdk->index, sdk->total));
    return !sendfaild_;
}

bool HLcdSdkProtocol::SendNotSDKData(const HCatBuffer &data, const HCatAny &userData)
{
    const HHeader *header = reinterpret_cast<const HHeader *>(data.ConstData());
    ParseLog(fmt::format("send-{}", HCmdType::HCmdTypeStr(header->cmd)));
    return SendSourceData(data, userData);
}

bool HLcdSdkProtocol::SendSourceData(const HCatBuffer &data, const HCatAny &userData)
{
    NotifyCmd(kSendData, data, userData);
    return !sendfaild_;
}

void HLcdSdkProtocol::NotifyReadyData(const HCatBuffer &data, const HCatAny &userData)
{
    NotifyCmd(kReadyRead, data, userData);
}

void HLcdSdkProtocol::NotifySendFileStatus(const HCatBuffer &data, const HCatAny &userData)
{
    NotifyCmd(kUploadFileProgress, data, userData);
}

void HLcdSdkProtocol::NotifyReset()
{
    NotifyCmd(kSendFaildAnswer, HCatBuffer(), HCatAny());
}

bool HLcdSdkProtocol::ParseReadData(const HCatBuffer &data, const HCatAny &userData)
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
    case HCmdType::kTcpHeartbeatAsk: {
        result = this->ParseHeartBeat(false);
    } break;
    case HCmdType::kTcpHeartbeatAnswer:
        break;
    case HCmdType::kLCDServiceAnswer: {
        result = this->ParseLCDServiceAnswer(buff);
    } break;
    case HCmdType::kLCDMsgAnswer: {
        result = this->ParseLCDMsgAnswer(buff);
    } break;
    case HCmdType::kErrorAnswer: {
        const HErrorStatus *status = reinterpret_cast<const HErrorStatus *>(buff.ConstData());
        processing_ = false;
        ParseLog(fmt::format("read error code[{}], status[{}]", HCmdType::HCmdTypeStr(status->head.cmd), HErrorCode::HErrorCodeStr(status->status)));
        NotifyReadyData(HCatBuffer(),  HReadProtocolData(HReadProtocolData::kErrorStatus, status->status));
    } break;
    case HCmdType::kFileListStartAnswer: {
        const HFileListStartAnswer *status = reinterpret_cast<const HFileListStartAnswer *>(buff.ConstData());
        if (status->status != 0) {
            ParseLog(fmt::format("read error code[{}], status[{}]", HCmdType::HCmdTypeStr(status->cmd), HErrorCode::HErrorCodeStr(status->status)));
            NotifyReadyData(HCatBuffer(), HReadProtocolData(HReadProtocolData::kErrorStatus, status->status));
            processing_ = false;
            result = this->DelaySend();
        } else {
            result = this->ParseSendFileListAnswer(userData);
        }
    } break;
    case HCmdType::kFileListEndAnswer: {
        hint64 fileSize = GetFileSize(lastFile_);
        this->NotifySendFileStatus(fmt::format("{};;{};;{};;2", lastFile_, fileSize, fileSize));
        processing_ = false;
        result = this->DelaySend();
    } break;
    case HCmdType::kFileStartAnswer: {
        result = this->ParseFileStartAnswer(data);
    } break;
    case HCmdType::kFileContentAnswer:
        break;
    case HCmdType::kFileEndAnswer: {
        if (sendFileList_.fileList.empty() == false) {
            lastFile_ = sendFileList_.fileList.front().filePath;
            sendFileList_.fileList.pop_front();
            hint64 fileSize = GetFileSize(lastFile_);
            this->NotifySendFileStatus(fmt::format("{};;{};;{};;3", lastFile_, fileSize, fileSize));
        }

        result = ParseSendFileListAnswer(userData);
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

bool HLcdSdkProtocol::ParseNegotiate()
{
    HTcpVersion version;
    version.len = sizeof(HTcpVersion);
    version.cmd = HCmdType::kLCDServiceAsk;
    version.version = LCD_VERSION;

    HCatBuffer data = HTool::iconv(version.len);
    data.Append(HTool::iconv(version.cmd));
    data.Append(HTool::iconv(version.version));

    if (SendNotSDKData(data) == false) {
        this->NotifyReset();
        ParseLog("Negotiate faild.");
        return false;
    }

    return true;
}

bool HLcdSdkProtocol::ParseLCDServiceAnswer(const HCatBuffer &data)
{
    if (data.Size() < sizeof(HTcpVersion)) {
        return false;
    }

    const HTcpVersion *version = reinterpret_cast<const HTcpVersion *>(data.ConstData());
    ParseLog(fmt::format("Lcd cmd[{}], version[{}]", HCmdType::HCmdTypeStr(version->cmd), version->version));
    if (version->version < LCD_VERSION) {
        return false;
    }

    NotifyCmd(kNotofyConnect, std::string(), HCatAny());

    // 完成协商, 开始进行延迟发送
    negotiate_ = true;
    return DelaySend();
}

bool HLcdSdkProtocol::ParseLCDMsgAnswer(HCatBuffer &data)
{
    HSDKCmd sdkCmd(data.GetString(sizeof(HSDKCmd)));
    data.Remove(0, sizeof(HSDKCmd));

    if (data.Empty() == false) {
        sdkBuff_.Append(data);
        if (sdkCmd.total <= sdkBuff_.Size()) {
            this->NotifyReadyData(sdkBuff_);
            sdkBuff_.Clear();
        }
    }

    return true;
}

bool HLcdSdkProtocol::ParseHeartBeat(bool ask)
{
    HHeader heartbeat(sizeof(HHeader), ask ? HCmdType::kTcpHeartbeatAsk : HCmdType::kTcpHeartbeatAnswer);
    HCatBuffer data;
    data.Append(HTool::iconv(heartbeat.len));
    data.Append(HTool::iconv(heartbeat.cmd));
    return SendNotSDKData(data);
}

bool HLcdSdkProtocol::ParseSendSDK(hint32 cmd, const HCatBuffer &data, const HCatAny &userData)
{
    // 还未协商完, 完成后会自动补发
    if (negotiate_ == false) {
        delaySend_.emplace_back(delaySend(cmd, data, userData));
        return true;
    }

    if (data.Empty()) {
        return true;
    }

    auto queue = SplitSdkData(data);
    for (auto &i : queue) {
        if (SendData(i, userData) == false) {
            this->NotifyReset();
            return false;
        }
    }

    return true;
}

bool HLcdSdkProtocol::SendFileListStart(const HCatBuffer &data, const HCatAny &userData)
{
    (void)data;
    if (userData.IsType<HSendProtocolFileInfo>() == false) {
        ParseLog(fmt::format("Send file list type error[{}]", userData.Type().name()));
        return false;
    }

    sendFileList_ = userData.Cast<HSendProtocolFileInfo>();
    if (sendFileList_.fileList.empty()) {
        return false;
    }

    std::string guid;
    if (sendFileList_.userData.IsType<std::string>()) {
        guid = sendFileList_.userData.Cast<std::string>();
    }

    if (guid.empty()) {
        guid = HTool::Uuid();
    }

    std::vector<char> fileListStart(sizeof(HFileListStartAsk) + guid.size() + sizeof(char), 0);
    HFileListStartAsk *ask = reinterpret_cast<HFileListStartAsk *>(fileListStart.data());
    ask->len = static_cast<huint16>(fileListStart.size());
    ask->cmd = HCmdType::kFileListStartAsk;
    ask->fileCount = static_cast<huint16>(sendFileList_.fileList.size());
    ask->AllFileSize = 0;
    for (const auto &i : sendFileList_.fileList) {
        ask->AllFileSize += GetFileSize(i.filePath);
    }
    memcpy(ask->sessionID, guid.data(), guid.size());

    return SendNotSDKData(std::string(fileListStart.data(), fileListStart.size()));
}

bool HLcdSdkProtocol::ParseSendFileListAnswer(const HCatAny &userData)
{
    if (sendFileList_.fileList.empty()) {
        return this->SendFileListEnd();
    }

    threadRun_ = !core_.expired();
    ICatEventApi::AddThread(core_, this, &HLcdSdkProtocol::SendFileStart, sendFileList_.fileList.front(), userData);
    return true;
}

bool HLcdSdkProtocol::SendFileListEnd()
{
    std::vector<char> data(sizeof(HHeader), 0);
    HHeader *ask = reinterpret_cast<HHeader *>(data.data());
    ask->len = static_cast<huint16>(data.size());
    ask->cmd = HCmdType::kFileListEndAsk;

    return SendNotSDKData(HCatBuffer(data.data(), data.size()));
}

bool HLcdSdkProtocol::SendFileStart(const HSendFile &sendFile, const HCatAny &userData)
{
    RallPoint<bool> threadRun(&threadRun_, false);
    if (sendfaild_) {
        return false;
    }

    std::string filePath = sendFile.filePath;
    std::string md5 = sendFile.md5;
    if (md5.empty()) {
        md5 = HTool::MD5_U8(filePath, &sendfaild_);
        if (sendfaild_) {
            return false;
        }
    }

    if (md5.empty()) {
        ParseLog(fmt::format("File cannot be opened. file[{}]", filePath));
        return false;
    }

    std::string fileName = filePath.substr(filePath.find_last_of("/\\") + 1);
    if (sendFile.fileName.empty() == false) {
        fileName = sendFile.fileName;
    }
    std::string fileAsk(fileName.size() + sizeof(HFileStartAsk) - sizeof(hint8), '\0');

    HFileStartAsk *ask = reinterpret_cast<HFileStartAsk *>(&fileAsk.front());
    ask->len = static_cast<huint16>(fileAsk.size());
    ask->cmd = HCmdType::kFileStartAsk;
    ask->size = GetFileSize(filePath);
    strcpy(ask->md5, md5.data());
    strcpy(ask->name, fileName.data());
    ask->type = sendFile.type;

    if (SendNotSDKData(fileAsk, userData) == false) {
        this->NotifyReset();
        return false;
    }

    return true;
}

bool HLcdSdkProtocol::ParseFileStartAnswer(const HCatBuffer &data)
{
    do {
        if (sendFileList_.fileList.empty()) {
            ParseLog("Sending file name is empty.");
            return false;
        }

        const HFileStartAnswer *answer = reinterpret_cast<const HFileStartAnswer *>(data.ConstData());
        if (answer->status != 0) {
            ParseLog(fmt::format("read status error[{}]", answer->status));
            processing_ = false;
            return DelaySend();
        }

        lastFile_ = sendFileList_.fileList.front().filePath;
        threadSendFilePtr callBack(new FileCallBack);
        callBack->sendFile.open(IconvFilePath(lastFile_), std::ios::in | std::ios::binary);
        if (callBack->sendFile.is_open() == false) {
            ParseLog(fmt::format("open file[{}] faild", lastFile_));
            break;
        }

        callBack->sendFile.seekg(0, std::ios::end);
        hint64 fileSize = callBack->sendFile.tellg();
        callBack->sendFile.seekg(0, std::ios::beg);
        hint64 sendSize = answer->existSize;

        ParseLog(fmt::format("File exists size[{}]", sendSize));
        if (fileSize == sendSize) {
            ParseLog(fmt::format("File already exists. file[{}]", lastFile_));
            NotifySendFileStatus(fmt::format("{};;{};;{};;0", lastFile_, sendSize, fileSize));
            break;
        }

        callBack->GetSendData(sendSize);
        callBack->filePath = lastFile_;
        callBack->fileSize = fileSize;
        callBack->sendSize = sendSize;
        if (callBack->sendData.Empty()) {
            break;
        }

        if (SendNotSDKData(callBack->sendData) == false) {
            ParseLog(fmt::format("File sending failed. file[{}]. send size[{}]", lastFile_, sendSize));
            return false;
        }

        callBack->sendSize += callBack->readSize;
        NotifySendFileStatus(fmt::format("{};;{};;{};;0", callBack->filePath, callBack->sendSize, fileSize));
        ParseLog(fmt::format("[{}] sending size[{}]/[{}]", callBack->filePath, callBack->sendSize, fileSize));
        threadRun_ = !core_.expired();
        ICatEventApi::AddThread(core_, this, &HLcdSdkProtocol::ThreadSendFile, HCatAny(callBack));
        return true;

    } while (false);

    return this->SendFileEnd();
}

bool HLcdSdkProtocol::ThreadSendFile(const HCatAny &userData)
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
            ICatEventApi::Forward(core_, this, &HLcdSdkProtocol::NotifySendFileStatus, fmt::format("{};;{};;{};;1", callBack->filePath, callBack->sendSize, callBack->fileSize), userData);
            return false;
        }

        callBack->sendSize += callBack->readSize;
        ICatEventApi::Forward(core_, this, &HLcdSdkProtocol::NotifySendFileStatus, fmt::format("{};;{};;{};;0", callBack->filePath, callBack->sendSize, callBack->fileSize), userData);
    }

    return SendFileEnd();
}

bool HLcdSdkProtocol::SendFileEnd()
{
    char sendEnd[sizeof(HHeader)];
    HHeader *ask = reinterpret_cast<HHeader *>(sendEnd);
    ask->len = sizeof(HHeader);
    ask->cmd = HCmdType::kFileEndAsk;
    if (SendNotSDKData(HCatBuffer(sendEnd, sizeof(HHeader))) == false) {
        this->NotifyReset();
        ParseLog("Send file End request faild");
        return false;
    }
    return true;
}

bool HLcdSdkProtocol::DelaySend()
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

hint64 HLcdSdkProtocol::GetFileSize(const std::string &filePath)
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

std::string HLcdSdkProtocol::IconvFilePath(const std::string &filePath) const
{
#ifdef H_WIN
    return cat::HCatTool::UTF8ToANSI(filePath);
#else
    return filePath;
#endif
}

void HLcdSdkProtocol::ParseLog(const std::string &log)
{
#ifdef DEBUG_LOG_SIGNAL
    ICatEventApi::ForwardSignal(core_, this, DebugLog, log);
#endif
}

bool HLcdSdkProtocol::FileCallBack::GetSendData(hint64 offset)
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
