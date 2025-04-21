

#include <ICApi.h>
#include <IEventApi.h>
#include <HCatSyncCoreEvent.h>
#include <HCApiImpl.h>
#include <HCatSignalSet.h>
#include <Protocol/HStruct.h>


#define H_UNUSER(x) (void)x


using namespace cat;

struct HBackCall
{
    int type;
    void *callBack;
    void *userData;
    HBackCall() : type(-1), callBack(nullptr), userData(nullptr) {}
    explicit HBackCall(int type_, void *callbakc_ = nullptr, void *userData_ = nullptr) : type(type_), callBack(callbakc_), userData(userData_) {}
};

struct HBackCallList
{
    HCatSignalSet slot;             ///< 会话连接槽
    std::list<HBackCall> backcall;  ///< 回调集合
    int type;                       ///< 连接类型
    HBackCallList() : type(-1) {}
};
using BackCallPtr = std::shared_ptr<HBackCallList>;


///< 查找回调, 用于增加回调
static HBackCall &FindBackCall(const BackCallPtr &data, int type, int newType) {
    for (auto &i : data->backcall) {
        if (i.type != type) {
            continue;
        }

        return i;
    }

    data->backcall.emplace_back(HBackCall(newType));
    return data->backcall.back();
}


///< 通知相应回调
template <typename _Func, typename ..._Args>
void ParseNotify(ISession *currSession, int notType, _Args &&...args) {
    if (currSession->GetUserData().IsType<BackCallPtr>() == false) {
        return ;
    }

    BackCallPtr back = currSession->GetUserData().Cast<BackCallPtr>();
    for (auto &i : back->backcall) {
        if (i.type != notType) {
            continue;
        }

        if (!i.callBack) {
            continue;
        }

        reinterpret_cast<_Func>(i.callBack)(currSession, std::forward<_Args>(args)..., i.userData);
    }
}

static void NotifyConnect(ISession *currSession) {
    ParseNotify<NetStatusCallBack>(currSession, kNetStatusFunc, kConnect);
}

static void NotifyDisconnect(ISession *currSession) {
    ParseNotify<NetStatusCallBack>(currSession, kNetStatusFunc, kDisconnect);
}

static void NotifyUploadFileProgress(const std::string &filePath, hint64 sendSize, hint64 fileSize, int status, ISession *currSession) {
    ParseNotify<UploadFileProgressCallBack>(currSession, kUploadFileProgressFunc, filePath.data(), sendSize, fileSize, eUploadFileStatus(status));
}

static void NotifyReadyRead(const std::string &data, const HCatAny &protocolData, ISession *currSession) {
    do {
        if (protocolData.IsType<HReadProtocolData>() == false) {
            break;
        }

        const HReadProtocolData &proto = protocolData.Cast<HReadProtocolData>();
        if (proto.type == HReadProtocolData::kErrorStatus) {
            if (proto.data.IsNUll()) {
                return ;
            }

            ParseNotify<ErrorCodeCallBack>(currSession, kErrorCodeFunc, proto.data.Cast<huint16>());
            return ;
        }

        if (proto.type != HReadProtocolData::kDetectDevice) {
            break;
        }

        std::string id;
        std::string ip;
        if (proto.data.IsType<HProbeInfo>()) {
            const HProbeInfo &info = proto.data.Cast<HProbeInfo>();
            id = info.id;
            ip = info.ip;
        } else if (proto.data.IsType<HUpdateDevInfoStrAsk>()) {
            const HUpdateDevInfoStrAsk &info = proto.data.Cast<HUpdateDevInfoStrAsk>();
            id = info.id;
            ip = info.ip;
        } else {
            return ;
        }

        if (id.empty() || ip.empty()) {
            return ;
        }

        ParseNotify<DeviceInfoCallBack>(currSession, kDeviceInfoFunc, id.data(), id.size(), ip.data(), ip.size(), data.data(), data.size());
    } while (false);

    if (data.empty()) {
        return ;
    }

    ParseNotify<ReadyReadCallBack>(currSession, kReadyReadFunc, data.data(), data.size());
}

static void NotifyDebug(const std::string &log, ISession *currSession) {
    if (log.empty()) {
        return ;
    }

    ParseNotify<LogOutCallBack>(currSession, kDebugLogFunc, log.data(), log.size());
}


static void NotifyNewConnect(const std::shared_ptr<ISession> &newSession, ISession *currSession) {
    if (!currSession) {
        return ;
    }

    if (currSession->GetUserData().IsType<BackCallPtr>() == false) {
        return ;
    }

    BackCallPtr back = currSession->GetUserData().Cast<BackCallPtr>();

    BackCallPtr data(new HBackCallList());
    data->type = back->type;
    data->slot += newSession->Connected.Bind(NotifyConnect);
    data->slot += newSession->Disconnected.Bind(NotifyDisconnect);
    data->slot += newSession->ReadyRead.Bind(NotifyReadyRead);
    data->slot += newSession->UploadFileProgress.Bind(NotifyUploadFileProgress);
    data->slot += newSession->NewConnect.Bind(NotifyNewConnect);
    data->slot += newSession->DebugLog.Bind(NotifyDebug);
    newSession->SetUserData(data);

    CApiImpl()->AppendNetSession(newSession);
    ParseNotify<NewConnect>(currSession, kNewConnectFunc, newSession.get());
}



BASE_API HEventCore * DLL_CALL CreateEventCore()
{
    return CApiImpl()->CreateEventCore();
}

BASE_API void DLL_CALL FreeEventCore(const cat::HEventCore *core)
{
    CApiImpl()->FreeEventCore(core);
}

BASE_API HBool DLL_CALL Exec(const cat::HEventCore *core)
{
    auto p = CApiImpl()->CreateSyncEventCore(core);
    if (!p) {
        return HFalse;
    }

    return p->Exec();
}

BASE_API HBool DLL_CALL Run(const cat::HEventCore *core)
{
    auto p = CApiImpl()->CreateSyncEventCore(core);
    if (!p) {
        return HFalse;
    }

    return p->Run();
}

BASE_API void DLL_CALL Quit(const cat::HEventCore *core)
{
    CApiImpl()->FreeSyncEventCore(core);
}

BASE_API HSession * DLL_CALL CreateNetSession(const cat::HEventCore *core, eNetProtocol protocol)
{
    HSession *net = CApiImpl()->CreateNetSession(core);
    if (!net) {
        return nullptr;
    }

    switch (protocol) {
    case kSDK2:
    case kSDK2Service:
        net->SetProtocol(Protocol::HSDKProtocol);
        break;
    case kDetectDevice:
        net->SetProtocol(Protocol::HUdpFindDeviceProtocol);
        break;
    default:
        net->SetProtocol(Protocol::HSDKProtocol);
        break;
    }

    BackCallPtr data(new HBackCallList());
    data->type = protocol;
    data->slot += net->Connected.Bind(NotifyConnect);
    data->slot += net->Disconnected.Bind(NotifyDisconnect);
    data->slot += net->ReadyRead.Bind(NotifyReadyRead);
    data->slot += net->UploadFileProgress.Bind(NotifyUploadFileProgress);
    data->slot += net->NewConnect.Bind(NotifyNewConnect);
    data->slot += net->DebugLog.Bind(NotifyDebug);
    net->SetUserData(data);
    return net;
}

BASE_API void DLL_CALL FreeNetSession(cat::HSession *session)
{
    if (!session) {
        return ;
    }

    session->ClearUserData();
    CApiImpl()->FreeNetSession(session);
}

BASE_API int DLL_CALL SetNetSession(cat::HSession *session, int type, void *data)
{
    if (!session) {
        return HFalse;
    }

    if (session->GetUserData().IsType<BackCallPtr>() == false) {
        return HFalse;
    }

    BackCallPtr back = session->GetUserData().Cast<BackCallPtr>();

    // 设置规律是回调是偶数, 用户数据是奇数
    switch (type) {
    case kReadyReadFunc:
    case kNetStatusFunc:
    case kUploadFileProgressFunc:
    case kErrorCodeFunc:
    case kDeviceInfoFunc:
    case kNewConnectFunc:
    case kDebugLogFunc: {
        HBackCall &call = FindBackCall(back, type, type);
        call.callBack = data;
    } break;
    case kReadyReadUserData:
    case kNetStatusUserData:
    case kUploadFileProgressUserData:
    case kErrorCodeUserData:
    case kDeviceInfoUserData:
    case kNewConnectUserData:
    case kDebugLogUserData: {
        HBackCall &call = FindBackCall(back, type - 1, type - 1);
        call.userData = data;
    } break;
    default:
        return HFalse;
        break;
    }

    return HTrue;
}

BASE_API HBool DLL_CALL Isconnect(const cat::HSession *session)
{
    if (!session) {
        return HFalse;
    }

    return session->IsConnect();
}

BASE_API HBool DLL_CALL Connect(cat::HSession *session, const char *ip, int port)
{
    if (!session) {
        return HFalse;
    }

    if (session->GetUserData().IsType<BackCallPtr>() == false) {
        return HFalse;
    }

    const int type = session->GetUserData().Cast<BackCallPtr>()->type;
    switch (type) {
    case kDetectDevice:
        return session->ConnectUdp("", 0, "", 9527);
        break;
    case kSDK2Service:
        return session->Listen(port);
        break;
    case kSDK2:
    default:
        break;
    }

    return session->Connect(std::string(ip), port);
}

BASE_API void DLL_CALL Disconnect(cat::HSession *session)
{
    if (!session) {
        return ;
    }

    session->Disconnect();
}

BASE_API HBool DLL_CALL SendSDK(cat::HSession *session, const char *data, huint32 len)
{
    if (!session) {
        return HFalse;
    }

    return session->SendSDK(data, len);
}

BASE_API HBool DLL_CALL SendFile(cat::HSession *session, const char *filePath, int type)
{
    if (!session) {
        return HFalse;
    }

    SendFileListType fileList{ HSendFile(std::string(filePath), type) };
    return session->SendFile(fileList);
}
