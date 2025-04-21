

#ifndef __HCATTCPSERVICE_H__
#define __HCATTCPSERVICE_H__


#include <HCatNet/HCatTcpSocket.h>


namespace cat
{



class HCATEVENT_EXPORT HCatTcpService : public HCatEventObject
{
public:
    ///< 新连接将发送该信号, 请必须在触发这个信号后完成新连接信号绑定, 信号触发完毕后将自动进行连接的 Connect 初始化操作
    HSignal<void(std::shared_ptr<HCatTcpSocket>)> NewConnect;

    ///< 错误信息将发送该信号
    HSignal<void(const std::string &errorInfo)> ErrorInfo;

    ///< 成功连接将发送该信号
    HSignal<void()> Connected;

    ///< 断开连接将发送该信号
    HSignal<void()> Disconnected;

public:
    explicit HCatTcpService(const EventCoreWeakPtr &core);
    ~HCatTcpService();

    bool Listen(huint16 port);
    bool IsListen() const;
    void Close(bool signal = true);

private:
    void InitMember();
    void DestroyMember();

    void DisposeEvent(int fd);
    void NotifyError(std::string *log = nullptr);
    void ParseError(const std::string &error);
    void ParseNewConnect(int fd, const std::string &ip, huint16 port);

private:
    EventCoreWeakPtr core_;
#ifdef H_WIN
    SOCKET socket_;
    WSADATA wsaData_;
#else
    hint32 socket_;
#endif
    bool threadExit_;
    bool threadRun_;
};


}


#endif // __HCATTCPSERVICE_H__
