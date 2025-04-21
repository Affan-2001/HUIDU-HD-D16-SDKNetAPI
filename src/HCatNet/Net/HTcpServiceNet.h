

#ifndef __HTCPSERVICENET_H__
#define __HTCPSERVICENET_H__


#include <Net/INetBase.h>
#include <HCatSignalSet.h>

#include <memory>


namespace cat
{


class HCatTcpService;
class HCatTcpSocket;

class HTcpServiceNet : public INetBase
{
public:
    explicit HTcpServiceNet(const EventCoreWeakPtr &core);

    virtual ~HTcpServiceNet();
    virtual std::string Protocol() const override { return NetProtocol::HTcpServiceNet; }
    virtual HCatAny GetInfo() const override { return info_; }
    virtual void SetInfo(const HCatAny &info) override;
    virtual bool Connect(bool = true) override;
    virtual void Disconnect(bool signal = true) override;
    virtual bool Isconnect() const override;
    virtual bool Write(const HCatBuffer &data, const HCatAny &config = HCatAny()) override { (void)data; (void)config; return true; }

private:
    void InitSignal();
    void ParseNewConnect(std::shared_ptr<HCatTcpSocket> socket);

private:
    std::shared_ptr<HCatTcpService> socket_;
    TcpInfo info_;
    HCatSignalSet set_;
};


}


#endif // __HTCPSERVICENET_H__
