

#include <Net/HSerialNet.h>
#include <HCatNet/HCatSerial.h>


using namespace cat;


HSerialNet::HSerialNet(const EventCoreWeakPtr &core)
    : INetBase(core)
    , serial_(new HCatSerial(core))
{
    set_ += serial_->Connected.Bind(Connected);
    set_ += serial_->Disconnected.Bind(Disconnected);
    set_ += serial_->ErrorInfo.Bind<const std::string &>(DebugLog);
    set_ += serial_->ReadData.Bind<const HCatBuffer &>(ReadyRead);
}

HSerialNet::~HSerialNet()
{
    this->set_.Clear();
    this->serial_->Close(false);
}

void HSerialNet::SetInfo(const HCatAny &info)
{
    if (info.IsType<SerialInfo>() == false) {
        return ;
    }

    info_ = info.Value<SerialInfo>();
}

bool HSerialNet::Connect(bool async)
{
    (void)async;
    return serial_->Open(info_.com, info_.baudRate, info_.data, info_.parity, info_.stopBit);
}

void HSerialNet::Disconnect(bool signal)
{
    serial_->Close(signal);
}

bool HSerialNet::Isconnect() const
{
    return serial_->IsOpen();
}

bool HSerialNet::Write(const HCatBuffer &data, const HCatAny &config)
{
    (void)config;
    return serial_->SendData(data);
}
