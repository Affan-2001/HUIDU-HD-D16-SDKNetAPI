

#include <Protocol/HHttpProtocol.h>
#include <HCatNet/HCatTcpSocket.h>
#include <HCatTool.h>
#include <algorithm>
#include <fstream>
#include <cstdio>


#define STORE_TEMP_FILE_PATH    "./"


using namespace cat;

class FileCtl
{
public:
    explicit FileCtl(const std::string &path) : filePath(path) {
        if (path.empty()) {
            return ;
        }

#ifdef H_WIN
        std::string name = cat::HCatTool::UTF8ToANSI(path);
#else
        std::string name = path;
#endif
        file.open(name, std::ios::out | std::ios::binary);
    }
    ~FileCtl() {
        this->Close();
        if (filePath.empty() == false) {
            remove(filePath.data());
        }
    }

    void Append(const HCatBuffer &data) {
        if (file.is_open() == false) {
            return ;
        }

        file.write(data.ConstData(), data.Size());
    }

    bool IsOpen() const { return file.is_open(); }

    void Close() {
        if (file.is_open()) {
            file.close();
        }
    }

    const std::string &GetPath() const { return filePath; }

private:
    std::string filePath;
    std::fstream file;
};



HHttpProtocol::HHttpProtocol(const EventCoreWeakPtr &core)
    : IProtocolBase(core)
{
    this->Init();
}

HHttpProtocol::~HHttpProtocol()
{
    if (tempFile_) {
        tempFile_.reset();
    }
}

bool HHttpProtocol::Dispose(hint32 cmd, const HCatBuffer &data, const HCatAny &userData)
{
    switch (cmd) {
    case kInit: {
        this->Init();
        return true;
    } break;
    case kSendFaildAsk: {
        sendfaild_ = true;
        return true;
    } break;
    case kReset:
    case kHeartBeat: {
        return true;
    } break;
    case kConnect: {
        this->Init();
        NotifyCmd(kNotofyConnect, data, userData);
        this->negotiate_ = true;
        this->DelaySend();
        return true;
    } break;
    case kReadData: {
        return this->ParseReadData(data);
    } break;
    case kSendFile: {
        ParseLog("Send file not realized");
        return true;
    } break;
    case kSendSDK: {
        if (negotiate_ == false) {
            delaySend_.emplace_back(delaySend(cmd, data, userData));
            return true;
        }
        return this->ParseSendData(data, userData);
    } break;
    default:
        break;
    }

    return false;
}

HHttpRequest HHttpProtocol::CreateRequest(HHttpRequest::eType type, const std::string &url)
{
    HHttpRequest request(type, url);
    request.header["Connection"] = "keep-alive";
    request.header["User-Agent"] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/94.0.4606.81 Safari/537.36";
    request.header["Accept"] = "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9";
    request.header["Accept-Language"] = "zh-CN,zh-TW;q=0.9,zh;q=0.8,en-US;q=0.7,en;q=0.6";

    return request;
}

HHttpRequest &HHttpProtocol::AppRequest(HHttpRequest &dest, const HHttpRequest &src)
{
    dest.type = src.type;
    dest.port = src.port;
    if (src.url.empty() == false) {
        dest.url = src.url;
    }

    for (auto i = src.header.begin(); i != src.header.end(); ++i) {
        dest.header[i->first] = i->second;
    }
    return dest;
}

HCatBuffer HHttpProtocol::ToHttpHeader(const HHttpRequest &header)
{
    HCatBuffer data;

    // 请求行
    switch (header.type) {
    case HHttpRequest::kGet:
        data.Append("GET").Append(" ");
        break;
    case HHttpRequest::kHead:
        data.Append("HEAD").Append(" ");
        break;
    case HHttpRequest::kPost:
        data.Append("POST").Append(" ");
        break;
    case HHttpRequest::kPut:
        data.Append("PUT").Append(" ");
        break;
    case HHttpRequest::kDelete:
        data.Append("DELETE").Append(" ");
        break;
    case HHttpRequest::kConnect:
        data.Append("CONNECT").Append(" ");
        break;
    case HHttpRequest::kOptions:
        data.Append("OPTIONS").Append(" ");
        break;
    case HHttpRequest::kTrace:
        data.Append("TRACE").Append(" ");
        break;
    case HHttpRequest::kPatch:
        data.Append("PATCH").Append(" ");
        break;
    default:
        data.Append("GET").Append(" ");
        break;
    }

    std::string resource("/");
    if (HCatTcpSocket::IsVaildAddr(header.url) == false) {
        resource = HCatTcpSocket::UrlToResource(header.url);
        if (resource == header.url) {
            resource = "/";
        }
    }
    data.Append(resource).Append(" ");
    data.Append("HTTP/1.1").Append("\r\n");

    // 请求头部
    std::string hostIp(HCatTcpSocket::UrlToHost(header.url));
    if (HCatTcpSocket::IsVaildAddr(hostIp) == false) {
        auto ipList = HCatTcpSocket::UrlToIp(header.url);
        if (ipList.empty() == false) {
            hostIp = std::move(ipList.front());
        }
    }
    data.Append("Host").Append(":").Append(hostIp).Append(":").Append(HCatTool::ToType<std::string>(header.port)).Append("\r\n");
    for (auto i = header.header.begin(); i != header.header.end(); ++i) {
        data.Append(i->first).Append(":").Append(i->second).Append("\r\n");
    }

    return data.Append("\r\n");
}

HHttpResponse HHttpProtocol::ToHttpResponseHeader(const HCatBuffer &data)
{
    HHttpResponse response(0);
    HCatBuffer header = ExtractHeader(data);
    if (header.Empty()) {
        return response;
    }

    auto strList = HCatTool::Split(header.GetConstString(), "\r\n");
    if (strList.empty()) {
        return response;
    }

    auto head = HCatTool::Split(strList.at(0));
    strList.erase(strList.begin());
    if (head.size() > 1) {
        response.status = HCatTool::ToType<hint16>(head.at(1));
    }

    for (const auto &i : strList) {
        std::size_t sepPos = i.find(":");
        if (sepPos == std::string::npos) {
            continue;
        }

        std::string key = HCatTool::Simplified(i.substr(0, sepPos));
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        response.header[key] = HCatTool::Simplified(i.substr(sepPos + 1));
    }

    return response;
}

HCatBuffer HHttpProtocol::ExtractHeader(const HCatBuffer &data)
{
    std::size_t pos = data.Find("\r\n\r\n");
    if (pos == std::string::npos) {
        return data;
    }

    HCatBuffer header(data);
    return header.Remove(pos);
}

void HHttpProtocol::Init()
{
    this->response_.Clear();
    this->readData_.Clear();
    this->DataLen_ = 0;
    this->downLen_ = 0;
    this->sendfaild_ = false;
    this->negotiate_ = false;
}

bool HHttpProtocol::SendData(const HCatBuffer &data, const HCatAny &userData)
{
    NotifyCmd(kSendData, data, userData);
    return !sendfaild_;
}

bool HHttpProtocol::ParseSendData(const HCatBuffer &data, const HCatAny &userData)
{
    HHttpRequest request = CreateRequest(HHttpRequest::kGet, "/");
    if (data.Empty() == false) {
        request.header["Content-Length"] = HCatTool::ToType<std::string>(data.Size());
    }
    if (userData.IsType<HHttpRequest>()) {
        AppRequest(request, userData.Cast<HHttpRequest>());
    }

    HCatBuffer sendData = ToHttpHeader(request).Append(data);
    ParseLog(sendData.GetConstString());
    return SendData(sendData);
}

bool HHttpProtocol::ParseReadData(const HCatBuffer &data)
{
    if (data.Empty() == false) {
        readData_.Append(data);
    }

    if (response_.IsEmpty()) {
        std::size_t pos = readData_.Find("\r\n\r\n");
        if (pos == std::string::npos) {
            return true;
        }

        HCatBuffer header = ExtractHeader(readData_);
        ParseLog(header.GetConstString());
        response_ = ToHttpResponseHeader(header);
        readData_.Remove(0, pos + 4);

        // 检查到存在文件先删除
        if (tempFile_) {
            tempFile_.reset();
        }

        // 查看下是否存在内容长度
        if (response_.header.find("content-length") == response_.header.end()) {
            NotifyReadyData(readData_, HReadProtocolData(HReadProtocolData::kHttpResponse, response_));
            readData_.Clear();
            return true;
        }

        DataLen_ = HCatTool::ToType<hint64>(response_.header["content-length"]);

        // 文件类型不是文本内容那就进行文件存储
        if (IsHeaderType(response_.header, "Content-Type", "text/html") == false) {
            response_.dataStore = HHttpResponse::kFile;
            tempFile_.reset(new FileCtl(STORE_TEMP_FILE_PATH + HTool::Uuid()));
            downLen_ = 0;
        }
    }

    if (IsHeaderType(response_.header, "Transfer-Encoding", "chunked")) {
        NotifyReadyData(readData_, HReadProtocolData(HReadProtocolData::kHttpResponse, response_));
        readData_.Clear();
        return true;
    }

    if (downLen_ + static_cast<hint64>(readData_.Size()) >= DataLen_) {
        huint32 len = readData_.Size() - static_cast<huint32>(downLen_ + readData_.Size() - DataLen_);
        // 区分是文件转发还是数据转发
        if (tempFile_ && tempFile_->IsOpen()) {
            tempFile_->Append(readData_.Mid(0, len));
            tempFile_->Close();
            NotifyReadyData(tempFile_->GetPath(), HReadProtocolData(HReadProtocolData::kHttpResponse, response_));
            ICatEventApi::Gc(core_, std::move(tempFile_));
            tempFile_.reset();
        } else {
            NotifyReadyData(readData_.Mid(0, len), HReadProtocolData(HReadProtocolData::kHttpResponse, response_));
        }
        downLen_ = 0;
        readData_.Remove(0, len);
        response_.Clear();
    }

    // 文件转发时, 需要将内容都丢入文件
    if (tempFile_ && tempFile_->IsOpen()) {
        downLen_ += readData_.Size();
        tempFile_->Append(readData_);
        readData_.Clear();
    }

    if (readData_.Empty() == false) {
        ParseReadData(HCatBuffer());
    }
    return true;
}

void HHttpProtocol::NotifyReadyData(const HCatBuffer &data, const HCatAny &userData)
{
    NotifyCmd(kReadyRead, data, userData);
}

void HHttpProtocol::NotifyReset()
{
    NotifyCmd(kSendFaildAnswer, HCatBuffer(), HCatAny());
}

void HHttpProtocol::ParseLog(const std::string &log)
{
#ifdef DEBUG_LOG_SIGNAL
    ICatEventApi::ForwardSignal(core_, this, DebugLog, log);
#endif
}

bool HHttpProtocol::IsHeaderType(const AttributeTable &table, std::string key, std::string valus)
{
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    auto ptr = table.find(key);
    if (ptr == table.end()) {
        return false;
    }

    std::string readType = ptr->second;
    std::transform(readType.begin(), readType.end(), readType.begin(), ::tolower);
    std::transform(valus.begin(), valus.end(), valus.begin(), ::tolower);
    return readType.find(valus) != std::string::npos;
}

bool HHttpProtocol::DelaySend()
{
    std::list<delaySend> sendList;
    sendList.swap(delaySend_);

    for (auto &&i : sendList) {
        if (Dispose(i.cmd, i.data, i.userData) == false) {
            return false;
        }
    }

    return true;
}
