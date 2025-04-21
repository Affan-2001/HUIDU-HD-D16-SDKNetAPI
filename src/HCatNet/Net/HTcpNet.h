

#ifndef __HTCPNET_H__
#define __HTCPNET_H__


#include <Net/INetBase.h>
#include <HCatSignalSet.h>

#include <memory>


namespace cat
{


class HCatTcpSocket;

class HCATNET_EXPORT HTcpNet : public INetBase
{
public:
    explicit HTcpNet(const EventCoreWeakPtr &core);
    explicit HTcpNet(const EventCoreWeakPtr &core, const std::shared_ptr<HCatTcpSocket> &socket);

    virtual ~HTcpNet();
    virtual std::string Protocol() const override { return NetProtocol::HTcpNet; }
    virtual HCatAny GetInfo() const override { return info_; }
    virtual void SetInfo(const HCatAny &info) override;
    virtual bool Connect(bool async = true) override;
    virtual void Disconnect(bool signal = true) override;
    virtual bool Isconnect() const override;
    virtual bool Write(const HCatBuffer &data, const HCatAny &config = HCatAny()) override;

private:
    void InitSignal();

private:
    std::shared_ptr<HCatTcpSocket> socket_;
    TcpInfo info_;
    HCatSignalSet set_;
};


}


#endif // __HTCPNET_H__
