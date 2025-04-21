

#include <Protocol/HRawStringProtocol.h>
#include <sstream>


using namespace cat;


HRawStringProtocol::HRawStringProtocol(const EventCoreWeakPtr &core)
    : IProtocolBase(core)
    , sendfaild_(false)
    , negotiate_(false)
{

}

HRawStringProtocol::~HRawStringProtocol()
{

}

bool HRawStringProtocol::Dispose(hint32 cmd, const HCatBuffer &data, const HCatAny &userData)
{
    switch (cmd) {
    case kInit: {
        sendfaild_ = false;
        negotiate_ = false;
        delaySend_.clear();
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
        NotifyCmd(kReadyRead, data, userData);
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
        return this->SendData(data, userData);
    } break;
    default:
        break;
    }

    return false;
}

bool HRawStringProtocol::SendData(const HCatBuffer &data, const HCatAny &userData)
{
    NotifyCmd(kSendData, data, userData);
    return !sendfaild_;
}

void HRawStringProtocol::ParseLog(const std::string &log)
{
#ifdef DEBUG_LOG_SIGNAL
    ICatEventApi::ForwardSignal(core_, this, DebugLog, log);
#endif
}

bool HRawStringProtocol::DelaySend()
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
