

#include <Net/HTcpServiceNet.h>
#include <Net/HTcpNet.h>
#include <HCatNet/HCatTcpService.h>


using namespace cat;


HTcpServiceNet::HTcpServiceNet(const EventCoreWeakPtr &core)
    : INetBase(core)
    , socket_(new HCatTcpService(core))
{
    this->InitSignal();
}

HTcpServiceNet::~HTcpServiceNet()
{
    this->set_.Clear();
    this->socket_->Close(false);
}

void HTcpServiceNet::SetInfo(const HCatAny &info)
{
    if (info.IsType<TcpInfo>() == false) {
        return ;
    }

    info_ = info.Value<TcpInfo>();
}

bool HTcpServiceNet::Connect(bool)
{
    return socket_->Listen(info_.port);
}

void HTcpServiceNet::Disconnect(bool signal)
{
    socket_->Close(signal);
}

bool HTcpServiceNet::Isconnect() const
{
    return socket_->IsListen();
}

void HTcpServiceNet::InitSignal()
{
    set_.Clear();

    set_ += socket_->Connected.Bind(Connected);
    set_ += socket_->Disconnected.Bind(Disconnected);
    set_ += socket_->ErrorInfo.Bind<const std::string &>(DebugLog);
    set_ += socket_->NewConnect.Bind(this, &HTcpServiceNet::ParseNewConnect);
}

void HTcpServiceNet::ParseNewConnect(std::shared_ptr<HCatTcpSocket> socket)
{
    if (!socket) {
        return ;
    }

    std::shared_ptr<HTcpNet> net(new HTcpNet(core_, socket));
    NetConnect(net);
}
