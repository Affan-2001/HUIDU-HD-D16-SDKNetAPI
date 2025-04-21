

#ifndef __HCATTCPSOCKET_H__
#define __HCATTCPSOCKET_H__


#include <HCatEventCore.h>
#include <HCatEventStruct.h>
#include <HCatThread.h>
#include <HCatSignal.h>
#include <HCatBuffer.h>

#ifdef H_WIN
#include <WinSock2.h>
#endif


namespace cat
{


class HCATEVENT_EXPORT HCatTcpSocket : public HCatEventObject
{
public:
    ///< 数据接收完的时候将发送该信号
    HSignal<void(const HCatBuffer &data)> ReadData;

    ///< 成功连接将发送该信号
    HSignal<void()> Connected;

    ///< 断开连接将发送该信号
    HSignal<void()> Disconnected;

    ///< 连接错误信息将发送该信号
    HSignal<void(const std::string &errorInfo)> ErrorInfo;

public:
    explicit HCatTcpSocket(const EventCoreWeakPtr &core);
    explicit HCatTcpSocket(const EventCoreWeakPtr &core, int socket, const std::string &ip, huint16 port);
    virtual ~HCatTcpSocket();

    ///< 一种已连接的重新进行数据侦听初始化并发出connected信号
    ///< 常用于服务端产生的连接(服务端产生的连接是必须调用这个才会进行数据处理)
    bool Connect();
    bool Connect(const std::string &ip, huint16 port, bool threadRun = true);
    void Disconnect(bool signal = true);
    bool IsConnect() const;

    bool SendData(const HCatBuffer &data);

    std::string GetIp() const { return ip_; }
    huint16 GetPort() const { return port_; }

    static bool IsIpv4(const std::string &ip);
    static bool IsIpv6(const std::string &ip);
    static bool IsVaildAddr(const std::string &ip) { return IsIpv4(ip) || IsIpv6(ip); }

    static std::string UrlToHost(const std::string &url);
    static std::string UrlToResource(const std::string &url);
    static std::list<std::string> UrlToIp(const std::string &url);

private:
    void InitMember();
    void DestroyMember();

    void Close(bool updateSignal = true);
    void DisposeEvent(hint32 fd);
    void ParseSendData(const HCatBuffer &data);
    void ParseConnect(const std::string &ip, huint16 port, const HCatAny &data);
    bool ParseConnectSignal();
    void ParseError(const std::string &error);
    void NotifyError(std::string *log = nullptr);

private:
    EventCoreWeakPtr core_;
    HCatThread thread_;
#ifdef H_WIN
    SOCKET socket_;
    WSADATA wsaData_;
#else
    hint32 socket_;
#endif

    std::string ip_;
    hint16 port_;
    bool threadExit_;
    bool threadRun_;
};



}


#endif // HCATTCPSOCKET_H
