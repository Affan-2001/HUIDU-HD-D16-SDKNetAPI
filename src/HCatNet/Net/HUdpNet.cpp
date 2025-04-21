

#include <Net/HUdpNet.h>
#include <HCatNet/HCatUdpSocket.h>


using namespace cat;


HUdpNet::HUdpNet(const EventCoreWeakPtr &core)
    : INetBase(core)
    , socket_(new HCatUdpSocket(core))
{
    this->InitSignal();
}

HUdpNet::~HUdpNet()
{
    this->set_.Clear();
    this->socket_->Disconnect(false);
}

void HUdpNet::SetInfo(const HCatAny &info)
{
    if (info.IsType<UdpInfo>() == false) {
        return ;
    }

    info_ = info.Value<UdpInfo>();
}

bool HUdpNet::Connect(bool isBind)
{
    // 转换IP, 广播的应该是任意地址, 而不是广播地址
    if (info_.listenType == kSendBroadcast) {
        info_.listenType = kSendMulticast;
    }
    return socket_->Connect(HCatUdpSocket::IP(info_.sendIp, info_.sendType), info_.sendPort, info_.sendType, HCatUdpSocket::IP(info_.ip, info_.listenType), info_.port, isBind);
}

void HUdpNet::Disconnect(bool signal)
{
    socket_->Disconnect(signal);
}

bool HUdpNet::Isconnect() const
{
    return socket_->IsConnect();
}

bool HUdpNet::Write(const HCatBuffer &data, const HCatAny &config)
{
    (void)config;
    return socket_->SendData(data);
}

void HUdpNet::InitSignal()
{
    set_.Clear();

    set_ += socket_->Connected.Bind(Connected);
    set_ += socket_->Disconnected.Bind(Disconnected);
    set_ += socket_->ErrorInfo.Bind<const std::string &>(DebugLog);
    set_ += socket_->ReadData.Bind<const std::string &, huint16, const HCatBuffer &>(ReadyReadUdp);
}
