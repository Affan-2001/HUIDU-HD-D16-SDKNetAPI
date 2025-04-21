

#ifndef __HSERIALNET_H__
#define __HSERIALNET_H__


#include <Net/INetBase.h>
#include <HCatSignalSet.h>

#include <memory>


namespace cat
{


class HCatSerial;

class HCATNET_EXPORT HSerialNet : public INetBase
{
public:
    explicit HSerialNet(const EventCoreWeakPtr &core);

    virtual ~HSerialNet() override;
    virtual std::string Protocol() const override { return NetProtocol::HSerialNet; }
    virtual HCatAny GetInfo() const override { return info_; }
    virtual void SetInfo(const HCatAny &info) override;
    virtual bool Connect(bool async = true) override;
    virtual void Disconnect(bool signal = true) override;
    virtual bool Isconnect() const override;
    virtual bool Write(const HCatBuffer &data, const HCatAny &config = HCatAny()) override;

private:
    std::shared_ptr<HCatSerial> serial_;
    SerialInfo info_;
    HCatSignalSet set_;
};



}


#endif // __HSERIALNET_H__
