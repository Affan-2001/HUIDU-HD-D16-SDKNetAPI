

#include <ISession.h>
#include <HCatTimer.h>
#include <HCatTool.h>
#include <Net/HSerialNet.h>
#include <Net/HTcpNet.h>
#include <Net/HUdpNet.h>
#include <Net/HTcpServiceNet.h>
#include <Protocol/HSDKProtocol.h>
#include <Protocol/HOldSDKProtocol.h>
#include <Protocol/HHexProtocol.h>
#include <Protocol/HSerialSDKProtocol.h>
#include <Protocol/HRawStringProtocol.h>
#include <Protocol/HUdpFindDeviceProtocol.h>
#include <Protocol/HLcdSdkProtocol.h>
#include <Protocol/HHttpProtocol.h>


#define HEART_BEAT_TIMEOUT (30000)


using namespace cat;


ISession::ISession(const EventCoreWeakPtrType &core)
    : core_(core)
    , heartBeat_(new HCatTimer())
    , heartBeatTime_(HEART_BEAT_TIMEOUT)
{
    this->protocol_.reset(new HSDKProtocol(core));
}

ISession::ISession(const EventCoreWeakPtrType &core, const std::string &protocol)
    : ISession(core)
{
    this->SetProtocol(protocol);
}

ISession::ISession(const EventCoreWeakPtrType &core, const std::string &protocol, const NetBasePtrType &net)
    : ISession(core)
{
    this->net_ = net;
    this->SetProtocol(protocol);
}

ISession::~ISession()
{
    signalSet_.Clear();
}

std::string ISession::GetIp() const
{
    if (!net_) {
        return "";
    }

    HCatAny info = net_->GetInfo();
    if (info.IsType<INetBase::TcpInfo>()) {
        return info.Cast<INetBase::TcpInfo>().ip;
    } else if (info.IsType<INetBase::UdpInfo>()) {
        return info.Cast<INetBase::UdpInfo>().sendIp;
    } else if (info.IsType<INetBase::SerialInfo>()) {
        return info.Cast<INetBase::SerialInfo>().com;
    }

    return "";
}

huint16 ISession::GetPort() const
{
    if (!net_) {
        return 0;
    }

    HCatAny info = net_->GetInfo();
    if (info.IsType<INetBase::TcpInfo>()) {
        return info.Cast<INetBase::TcpInfo>().port;
    } else if (info.IsType<INetBase::UdpInfo>()) {
        return info.Cast<INetBase::UdpInfo>().sendPort;
    } else if (info.IsType<INetBase::SerialInfo>()) {
        return info.Cast<INetBase::SerialInfo>().baudRate;
    }

    return 0;
}

std::string ISession::GetProtocol() const
{
    if (!protocol_) {
        return "";
    }

    return protocol_->Protocol();
}


bool ISession::SetProtocol(const std::string &protocol)
{
    auto old = protocol_;
    if (protocol == Protocol::HSDKProtocol) {
        protocol_.reset(new HSDKProtocol(core_));
    } else if (protocol == Protocol::HOldSDKProtocol) {
        protocol_.reset(new HOldSDKProtocol(core_));
    } else if (protocol == Protocol::HSerialSDKProtocol) {
        protocol_.reset(new HSerialSDKProtocol(core_));
    } else if (protocol == Protocol::HHexRawProtocol) {
        protocol_.reset(new HHexProtocol(core_));
    } else if (protocol == Protocol::HRawStringProtocol) {
        protocol_.reset(new HRawStringProtocol(core_));
    } else if (protocol == Protocol::HUdpFindDeviceProtocol) {
        protocol_.reset(new HUdpFindDeviceProtocol(core_));
    } else if (protocol == Protocol::HLcdSdkProtocol) {
        protocol_.reset(new HLcdSdkProtocol(core_));
    } else if (protocol == Protocol::HHttpProtocol) {
        protocol_.reset(new HHttpProtocol(core_));
    } else {
        return false;
    }

    InitSignal();
    // 延迟回收, 避免是回调期间切换
    ICatEventApi::Gc(core_, std::move(old));
    return true;
}

std::list<std::string> ISession::GetProtocols()
{
    static const std::list<std::string> protocol = {
        Protocol::HOldSDKProtocol,
        Protocol::HSDKProtocol,
        Protocol::HHexRawProtocol,
        Protocol::HSerialSDKProtocol,
        Protocol::HRawStringProtocol,
        Protocol::HUdpFindDeviceProtocol,
        Protocol::HLcdSdkProtocol,
        Protocol::HHttpProtocol,
    };

    return protocol;
}

bool ISession::SetNet(const std::string &net)
{
    if (net == NetProtocol::HTcpNet) {
        SetNet(NetBasePtrType(new HTcpNet(core_)));
        return true;
    } else if (net == NetProtocol::HSerialNet) {
        SetNet(NetBasePtrType(new HSerialNet(core_)));
        return true;
    } else if (net == NetProtocol::HUdpNet) {
        SetNet(NetBasePtrType(new HUdpNet(core_)));
        return true;
    } else if (net == NetProtocol::HTcpServiceNet) {
        SetNet(NetBasePtrType(new HTcpServiceNet(core_)));
        return true;
    }

    return false;
}

void ISession::SetNet(const NetBasePtrType &net)
{
    auto old = net_;
    if (old) {
        old->Disconnect(false);
    }

    net_ = net;
    InitSignal();
    // 延迟回收, 避免是回调期间切换
    ICatEventApi::Gc(core_, std::move(old));
}

std::string ISession::GetNetProtocol() const
{
    if (!net_) {
        return "";
    }

    return net_->Protocol();
}

std::list<std::string> ISession::GetNetProtocols()
{
    static const std::list<std::string> nets = {
        NetProtocol::HTcpNet,
        NetProtocol::HSerialNet,
        NetProtocol::HUdpNet,
        NetProtocol::HTcpServiceNet,
    };

    return nets;
}

bool ISession::Connect(const std::string &ip, huint16 port, bool async)
{
    if (core_.expired()) {
        DebugLog("Event core is nullptr", this);
        return false;
    }

    SetNet(NetProtocol::HTcpNet);
    INetBase::TcpInfo info(ip, port);
    net_->SetInfo(info);
    return net_->Connect(async);
}

bool ISession::ConnectSerial(const std::string &com, eBaudRate baudRate, eDataBits data, eParity parity, eStopBits stopBit)
{
    if (core_.expired()) {
        DebugLog("Event core is nullptr", this);
        return false;
    }

    SetNet(NetProtocol::HSerialNet);
    INetBase::SerialInfo info(com, baudRate, data, parity, stopBit);
    net_->SetInfo(info);
    return net_->Connect();
}

bool ISession::ConnectUdp(const std::string &listenIp, huint16 listenPort, const std::string &sendIp, huint16 sendPort, eUdpSendType listType, eUdpSendType sendType, bool isBind)
{
    if (core_.expired()) {
        DebugLog("Event core is nullptr", this);
        return false;
    }

    SetNet(NetProtocol::HUdpNet);
    INetBase::UdpInfo info(listenIp, listenPort, sendIp, sendPort, listType, sendType);
    net_->SetInfo(info);
    return net_->Connect(isBind);
}

bool ISession::Listen(huint16 port)
{
    if (core_.expired()) {
        DebugLog("Event core is nullptr", this);
        return false;
    }

    SetNet(NetProtocol::HTcpServiceNet);
    INetBase::TcpInfo info("", port);
    net_->SetInfo(info);
    if (net_->Connect()) {
        ParseProtocol(IProtocolBase::kNotofyConnect, "", HCatAny());
        return true;
    }

    return false;
}

void ISession::Disconnect()
{
    if (!net_) {
        return ;
    }

    net_->Disconnect();
}

bool ISession::IsConnect() const
{
    if (!net_) {
        return false;
    }

    return net_->Isconnect();
}

bool ISession::SendRaw(std::string &&data)
{
    if (!net_ || IsConnect() == false) {
        return false;
    }

    return net_->Write(HCatBuffer(std::move(data)));
}

bool ISession::SendRaw(const std::string &data)
{
    if (!net_ || IsConnect() == false) {
        return false;
    }

    return net_->Write(HCatBuffer(data));
}

bool ISession::SendRaw(const char *data, size_t num)
{
    return SendRaw(std::string(data, num));
}


bool ISession::SendSDK(const std::string &data, const HCatAny &protocolData)
{
    if (!net_ || IsConnect() == false) {
        return false;
    }

    return NotifyProtocol(IProtocolBase::kSendSDK, data, ToSdkProtocolData(protocolData));
}

bool ISession::SendSDK(const char *data, size_t num, const HCatAny &protocolData)
{
    return SendSDK(std::string(data, num), protocolData);
}


bool ISession::SendFile(const SendFileListType &fileList, const HCatAny &protocolData)
{
    if (!net_ || IsConnect() == false) {
        return false;
    }

    // 避免多线程出现转发传递时转发到指针导致访问野指针字符串的问题
    HCatAny data(protocolData);
    if (data.IsType<const char *>()) {
        data = std::string(data.Cast<const char *>());
    } else if (data.IsType<char *>()) {
        data = std::string(data.Cast<char *>());
    }

    return NotifyProtocol(IProtocolBase::kSendFile, "", HSendProtocolFileInfo(fileList, data));
}


void ISession::SetHeartBeatTime(hint64 ms)
{
    this->heartBeatTime_ = ms;
    this->heartBeat_->Start(core_, ms);
}

void ISession::InitSignal()
{
    signalSet_.Clear();

    signalSet_ += this->heartBeat_->Timeout.Bind([this](){
        if (NotifyProtocol(IProtocolBase::kHeartBeat) == false) {
            this->heartBeat_->Stop();
        }
    });

    if (net_) {
#ifdef DEBUG_LOG_SIGNAL
        signalSet_ += net_->DebugLog.Bind([this](const std::string &log){ DebugLog(log, this); });
#endif
        signalSet_ += net_->Connected.Bind([this](){ NotifyProtocol(IProtocolBase::kConnect); });
        signalSet_ += net_->Disconnected.Bind([this](){
            NotifyProtocol(IProtocolBase::kSendFaildAsk);
            heartBeat_->Stop();
            Disconnected(this);
        });
        signalSet_ += net_->ReadyRead.Bind([this](const HCatBuffer &data) { ParseNetReadData(data.GetConstString()); });
        signalSet_ += net_->ReadyReadUdp.Bind([this](const std::string &ip, huint16 port, const HCatBuffer &data) { NotifyProtocol(IProtocolBase::kReadData, data.GetConstString(), INetBase::TcpInfo(ip, port)); });
        signalSet_ += net_->NetConnect.Bind([this](const std::shared_ptr<INetBase> &net) {
            std::shared_ptr<ISession> session(new ISession(core_, GetProtocol(), net));
            NewConnect(session, this);
        });
    }

    if (protocol_) {
#ifdef DEBUG_LOG_SIGNAL
        signalSet_ += protocol_->DebugLog.Bind([this](const std::string &log){ DebugLog(log, this); });
#endif
        signalSet_ += protocol_->NotifyCmd.Bind([this](hint32 cmd, const HCatBuffer &data, const HCatAny &userData) { ParseProtocol(cmd, data.GetConstString(), userData); });
    }

    ///< 通知协议重进入初始化
    NotifyProtocol(IProtocolBase::kInit);
}

void ISession::ParseNetReadData(const std::string &data)
{
    // 读取数据处理失败, 那么协议就必须重初始化了
    if (NotifyProtocol(IProtocolBase::kReadData, data) == false) {
        NotifyProtocol(IProtocolBase::kInit);
        NotifyProtocol(IProtocolBase::kReset);
    }
}

void ISession::ParseProtocol(hint32 cmd, const std::string &data, const HCatAny &userData)
{
    SetHeartBeatTime(heartBeatTime_);

    switch (cmd) {
    case IProtocolBase::kNotofyConnect: {
        Connected(this);
    } break;
    case IProtocolBase::kSendData: {
        if (!net_ || net_->Isconnect() == false) {
            NotifyProtocol(IProtocolBase::kInit);
            break;
        }

        if (!net_ || net_->Write(HCatBuffer(data), userData) == false) {
            NotifyProtocol(IProtocolBase::kSendFaildAsk);
        }
    } break;
    case IProtocolBase::kReadyRead: {
        ReadyRead(data, userData, this);
    } break;
    case IProtocolBase::kUploadFileProgress: {
        std::vector<std::string> status = HCatTool::Split(data, ";;");
        if (status.size() < 4) {
            return ;
        }

        UploadFileProgress(status.at(0), status.at(1).empty() ? 0 : HCatTool::ToType<hint64>(status.at(1)), status.at(2).empty() ? 0 : HCatTool::ToType<hint64>(status.at(2)), HCatTool::ToType<hint32>(status.at(3)), this);
    } break;
    case IProtocolBase::kSendFaildAnswer: {
        this->NotifyProtocol(IProtocolBase::kInit);
    }
    default:
        break;
    }
}

bool ISession::NotifyProtocol(hint32 cmd, const std::string &data, const HCatAny &userData)
{
    if (!protocol_) {
        return false;
    }

    switch (cmd) {
    case IProtocolBase::kInit:
    case IProtocolBase::kHeartBeat:
        break;
    case IProtocolBase::kSendSDK:
    case IProtocolBase::kSendFile:
        SetHeartBeatTime(heartBeatTime_);
        if (ICatEventApi::IsMainThread(core_)) {
            break;
        }

        ICatEventApi::Forward(core_, protocol_.get(), &IProtocolBase::Dispose, cmd, data, userData);
        return net_->Isconnect();
    default:
        SetHeartBeatTime(heartBeatTime_);
        break;
    }


    return protocol_->Dispose(cmd, data, userData);
}


HCatAny ISession::ToSdkProtocolData(const HCatAny &userData)
{
    if (GetProtocol() == Protocol::HHttpProtocol) {
        HCatAny data(userData);
        if (data.IsType<HHttpRequest>() == false) {
            data = HHttpRequest();
        }

        HHttpRequest &request = data.Cast<HHttpRequest>();
        if (request.url.empty()) {
            request.url = GetIp();
        }
        request.port = GetPort();
        return data;
    }

    return userData;
}
