

#ifndef __ISESSION_H__
#define __ISESSION_H__


///< 这是最少需要包含的5个头文件和ISession.h头文件
#include <HCatNetInfo.h>
#include <HCatSignal.h>
#include <HCatSignalSet.h>
#include <HCatAny.h>
#include <ICatEventData.h>


#include <memory>


namespace cat
{


class HCatTimer;
class INetBase;
class IProtocolBase;
class ICatEventBase;

/**
 * @brief The ISession class 多协议会话, 默认SDK2.0协议, 依赖于事件核心
 * 这是一个多线程读写安全的会话
 * 发送写数据到协议会通过同步进程来确保数据是从主进程到协议
 * 所有的网络写数据会通过核心事件队列转发到发送队列进行.
 * 所有的网络读信号会通过同步进程来触发信号
 */
class HCATNET_EXPORT ISession
{
public:
    using EventCoreWeakPtrType = std::weak_ptr<ICatEventBase>;
    using NetBasePtrType = std::shared_ptr<INetBase>;
    using ProtocolBasePtrType = std::shared_ptr<IProtocolBase>;

    enum eUploadFileStatus {
        kFileUploading,         ///< 文件上传中
        kFileUploadFaild,       ///< 文件上传失败
        kFileUploadSuccess,     ///< 文件上传成功(如果支持发送多文件只有最后发送完才会发出这个状态, 单文件则每次完成都发出)
        kFileUploadNext,        ///< 上传下一个文件
    } ;

public:
    ///< 网络连接, 完成协议协商后才会触发该信号
    HSignal<void(ISession *currSession)> Connected;

    ///< 网络断开, 析构不会抛出
    HSignal<void(ISession *currSession)> Disconnected;

    ///< Debug日志
    HSignal<void(const std::string &log, ISession *currSession)> DebugLog;

    ///< 文件上传进度, 传递一个函数指针 status 0正常, 1失败 2完成结束操作
    HSignal<void(const std::string &filePath, hint64 sendSize, hint64 fileSize, int status, ISession *currSession)> UploadFileProgress;

    ///< 有数据, data是读取的数据, protocolData是协议传递的数据, 一般是空数据, currSession是自身会话
    HSignal<void(const std::string &data, const HCatAny &protocolData, ISession *currSession)> ReadyRead;

    ///< 有新会话, 在Tcp服务端时触发该信号, 协议将是当前服务端设置的协议
    ///< 新会话信号结束后将进行网络Connect信号触发, 所以得在这个信号触发的回调进行相关的新会话信号连接
    HSignal<void(const std::shared_ptr<ISession> &newSession, ISession *currSession)> NewConnect;

public:
    explicit ISession(const EventCoreWeakPtrType &core);
    explicit ISession(const EventCoreWeakPtrType &core, const std::string &protocol);
    explicit ISession(const EventCoreWeakPtrType &core, const std::string &protocol, const NetBasePtrType &net);
    ~ISession();

    std::string GetIp() const;
    huint16 GetPort() const;

    ///< 当前协议
    std::string GetProtocol() const;

    ///< 切换协议, 会重新初始化协议
    bool SetProtocol(const std::string &protocol);

    ///< 获取当前支持的协议
    static std::list<std::string> GetProtocols();

    ///< 切换网络, 会重新初始化协议
    bool SetNet(const std::string &net);
    void SetNet(const NetBasePtrType &net);

    ///< 当前网络协议
    std::string GetNetProtocol() const;

    ///< 获取当前支持的网络
    static std::list<std::string> GetNetProtocols();

    ///< 连接TCP, 网络会自动切换到TCP
    bool Connect(const std::string &ip, huint16 port, bool async = true);

    ///< 连接串口, 网络会自动切换到串口
    bool ConnectSerial(const std::string &com, eBaudRate baudRate = kBaud9600, eDataBits data = kData8, eParity parity = kNoParity, eStopBits stopBit = kOneStop);

    ///< 连接UDP, 网络会自动切换到UDP, 侦听类型是广播或组播则任意地址, ip地址可空
    bool ConnectUdp(const std::string &listenIp, huint16 listenPort, const std::string &sendIp, huint16 sendPort, eUdpSendType listType = kSendBroadcast, eUdpSendType sendType = kSendBroadcast, bool isBind = true);

    ///< 侦听Tcp, 网络自动切换到TCP服务端
    bool Listen(huint16 port);

    ///< 断开连接, 会抛出 Disconnected 信号
    void Disconnect();

    ///< 连接状态
    bool IsConnect() const;

    ///< 不通过协议直接发送数据
    bool SendRaw(std::string &&data);
    bool SendRaw(const std::string &data);
    bool SendRaw(const char *data, std::size_t num);

    ///< 发送通过协议的数据, 会进入协议发送队列, 可不等Connected信号触发
    bool SendSDK(const std::string &data, const HCatAny &protocolData = HCatAny());
    bool SendSDK(const char *data, std::size_t num, const HCatAny &protocolData = HCatAny());

    /// 发送文件, 第二个为协议参数, 比如led的需要传递 std::string 的guid数据, 空则协议则自动处理
    bool SendFile(const SendFileListType &fileList, const HCatAny &protocolData = HCatAny());

    ///< 设置心跳时间, 默认30s, 就是30000ms, 当有数据读取或发送时重置心跳时间
    void SetHeartBeatTime(hint64 ms);

    ///< 辅助功能, 提供用户存储数据在回调时提取数据的便利性
    void SetUserData(const HCatAny &userData) { userData_ = userData; }
    void SetUserData(HCatAny &&userData) { userData_ = std::move(userData); }
    const HCatAny &GetUserData() const { return userData_; }
    void ClearUserData() { userData_.Clear(); }

    ///< 获取事件核心
    EventCoreWeakPtrType GetEventCore() const { return core_; }
private:
    void InitSignal();
    void ParseNetReadData(const std::string &data);
    void ParseProtocol(hint32 cmd, const std::string &data, const HCatAny &userData);
    bool NotifyProtocol(hint32 cmd, const std::string &data = "", const HCatAny &userData = HCatAny());

    ///< 根据协议自行转换内部协议数据
    HCatAny ToSdkProtocolData(const HCatAny &userData);

private:
    EventCoreWeakPtrType core_;                 ///< 事件核心, 依赖于这个保持多线程安全
    HCatSignalSet signalSet_;                   ///< 槽集
    HCatAny userData_;                          ///< 暂留数据, 提供给用户保留自身数据使用
    NetBasePtrType net_;                        ///< 当前使用的网络
    ProtocolBasePtrType protocol_;              ///< 使用的协议
    std::unique_ptr<HCatTimer> heartBeat_;      ///< 心跳定时器
    hint64 heartBeatTime_;                      ///< 心跳时间
};


}


#endif // __ISESSION_H__
