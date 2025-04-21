

#include <Protocol/HHexProtocol.h>
#include <Tool/HTool.h>
#include <sstream>
#include <cctype>


using namespace cat;


HHexProtocol::HHexProtocol(const EventCoreWeakPtr &core)
    : IProtocolBase(core)
    , sendfaild_(false)
    , negotiate_(false)
{

}

HHexProtocol::~HHexProtocol()
{

}

bool HHexProtocol::Dispose(hint32 cmd, const HCatBuffer &data, const HCatAny &userData)
{
    switch (cmd) {
    case kInit: {
        sendfaild_ = false;
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
        this->sendfaild_ = false;
        NotifyCmd(kNotofyConnect, data, userData);
        this->negotiate_ = true;
        return this->DelaySend();
    } break;
    case kReadData: {
        NotifyCmd(kReadyRead, HCatBuffer(HTool::ToHex(data.GetString(), " ")), userData);
        return true;
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
        return this->SendData(HCatBuffer(HTool::FromHex(data.GetString())), userData);
    } break;
    default:
        break;
    }

    return false;
}

bool HHexProtocol::SendData(const HCatBuffer &data, const HCatAny &userData)
{
    NotifyCmd(kSendData, data, userData);
    return !sendfaild_;
}

void HHexProtocol::ParseLog(const std::string &log)
{
#ifdef DEBUG_LOG_SIGNAL
    ICatEventApi::ForwardSignal(core_, this, DebugLog, log);
#endif
}

bool HHexProtocol::DelaySend()
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
