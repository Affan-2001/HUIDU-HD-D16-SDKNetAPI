

#include <Net/HTcpNet.h>
#include <HCatNet/HCatTcpSocket.h>
#include <ICatEventApi.h>


using namespace cat;


HTcpNet::HTcpNet(const EventCoreWeakPtr &core)
    : INetBase(core)
    , socket_(new HCatTcpSocket(core))
{
    this->InitSignal();
}

HTcpNet::HTcpNet(const EventCoreWeakPtr &core, const std::shared_ptr<HCatTcpSocket> &socket)
    : INetBase(core)
    , socket_(socket)
{
    this->InitSignal();
}

HTcpNet::~HTcpNet()
{
    this->set_.Clear();
    this->socket_->Disconnect(false);
}

void HTcpNet::SetInfo(const HCatAny &info)
{
    if (info.IsType<TcpInfo>() == false) {
        return ;
    }

    info_ = info.Value<TcpInfo>();
}

bool HTcpNet::Connect(bool async)
{
    std::string ip = info_.ip;
    auto ipList = socket_->UrlToIp(ip);
    if (ipList.empty() == false) {
        ip = std::move(ipList.front());
    }
    return socket_->Connect(ip, info_.port, async);
}

void HTcpNet::Disconnect(bool signal)
{
    socket_->Disconnect(signal);
}

bool HTcpNet::Isconnect() const
{
    return socket_->IsConnect();
}

bool HTcpNet::Write(const HCatBuffer &data, const HCatAny &config)
{
    (void)config;
    return socket_->SendData(data);
}

void HTcpNet::InitSignal()
{
    set_.Clear();

    set_ += socket_->Connected.Bind(Connected);
    set_ += socket_->Disconnected.Bind(Disconnected);
    set_ += socket_->ErrorInfo.Bind<const std::string &>(DebugLog);
    set_ += socket_->ReadData.Bind<const HCatBuffer &>(ReadyRead);
}
