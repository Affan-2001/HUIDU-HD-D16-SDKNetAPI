

#ifndef __HUDPNET_H__
#define __HUDPNET_H__



#include <Net/INetBase.h>
#include <HCatSignalSet.h>

#include <memory>


namespace cat
{


class HCatUdpSocket;

class HCATNET_EXPORT HUdpNet : public INetBase
{
public:
    explicit HUdpNet(const EventCoreWeakPtr &core);
    virtual ~HUdpNet() override;
    virtual std::string Protocol() const override { return NetProtocol::HUdpNet; }
    virtual HCatAny GetInfo() const override { return info_; }
    virtual void SetInfo(const HCatAny &info) override;
    virtual bool Connect(bool isBind = true) override;
    virtual void Disconnect(bool signal = true) override;
    virtual bool Isconnect() const override;
    virtual bool Write(const HCatBuffer &data, const HCatAny &config) override;

private:
    void InitSignal();

private:
    std::shared_ptr<HCatUdpSocket> socket_;
    UdpInfo info_;
    HCatSignalSet set_;
};


}


#endif // HUDPNET_H
