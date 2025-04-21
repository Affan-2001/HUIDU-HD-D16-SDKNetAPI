
#include <WinSock2.h>
#include <HCatNet/HCatUdpSocket.h>
#include <ICatEventApi.h>
#include <string.h>
#include <HCatTool.h>

#ifdef H_WIN
#include <HCatTool.h>

#include <windows.h>
#include <ws2ipdef.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define SOCKET_ERROR (-1)
#endif


#define BUFF_MAX    (1024 * 9)


using namespace cat;



HCatUdpSocket::HCatUdpSocket(const EventCoreWeakPtr &core)
    : core_(core)
    , socket_(SOCKET_ERROR)
    , sendIp_(INADDR_BROADCAST)
    , port_(0)
    , threadExit_(false)
    , threadRun_(false)
{
    this->InitMember();
}

HCatUdpSocket::~HCatUdpSocket()
{
    threadExit_ = true;
    this->DestroyMember();
    while (threadRun_ && !core_.expired()) {
        threadExit_ = true;
        std::this_thread::yield();
    }
}

HCatUdpSocket::IpType HCatUdpSocket::IP(const std::string &ip, eUdpSendType type)
{
    switch (type) {
    case kSendUnicast: {
        return inet_addr(ip.data());
    } break;
    case kSendMulticast: {
        return INADDR_ANY;
    } break;
    case kSendBroadcast: {
        return INADDR_BROADCAST;
    } break;
    default:
        return INADDR_ANY;
    }
}

bool HCatUdpSocket::Connect(IpType sendIp, huint16 sendPort, eUdpSendType sendType, IpType listenIp, huint16 listentPort, bool isBind)
{
    if (core_.expired()) {
        this->ParseError("Event core is nullptr");
        return false;
    }

    this->Close(false);
    this->port_ = sendPort;
    this->sendIp_ = sendIp;

    socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (SOCKET_ERROR == socket_){
        ParseError("create socket faild");
        return false;
    }

    switch (sendType) {
    case kSendMulticast: {
        // 加入多播组
        struct ip_mreq multcast;
        multcast.imr_multiaddr.s_addr = sendIp;
        multcast.imr_interface.s_addr = htonl(INADDR_ANY);
        setsockopt(socket_, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<char *>(&multcast), sizeof(multcast));
    } break;
    case kSendBroadcast: {
#ifdef H_WIN
        char option = 1;
#else
        int option = 1;
#endif
        setsockopt(socket_, SOL_SOCKET, SO_BROADCAST, &option, sizeof(option));
    } break;
    default:
        break;
    }

#ifdef H_WIN
    char opt = 1;
#else
    int opt = 1;
#endif
    setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

#ifdef SO_REUSEPORT
    opt = 1;
    setsockopt(socket_, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
#endif

    // 是否绑定
    if (isBind == false) {
        ICatEventApi::ForwardSignal(core_, this, Connected);
        return true;
    }

    sockaddr_in addrSrv;
    memset(&addrSrv, 0, sizeof(addrSrv));
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(listentPort);
    addrSrv.sin_addr.s_addr = listenIp;

    int ret = bind(socket_, reinterpret_cast<sockaddr *>(&addrSrv), sizeof(addrSrv));
    if (ret != 0) {
        std::string log;
        this->NotifyError(&log);
        this->Close();
        this->ParseError(log);
        return false;
    }

    return ParseConnectSignal();
}

void HCatUdpSocket::Disconnect(bool signal)
{
    this->Close(signal);
}

bool HCatUdpSocket::IsConnect() const
{
    return this->socket_ != SOCKET_ERROR;
}

bool HCatUdpSocket::SendData(const HCatBuffer &data)
{
    if (this->socket_ == SOCKET_ERROR) {
        return false;
    }

    threadRun_ = !core_.expired();
    ICatEventApi::AddIOReadWrite(core_, this, &HCatUdpSocket::ParseSendData, data);
    return this->IsConnect();
}

void HCatUdpSocket::NotifyError(std::string *log)
{
#ifdef H_WIN
        wchar_t *s = NULL;
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, WSAGetLastError(),
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       (LPWSTR)&s, 0, NULL);

        std::string err = HCatTool::UnicodeToUTF8(s);
        LocalFree(s);
        err.erase(err.find_last_not_of("\r\n") + 1);
        if (log) {
            *log = std::move(err);
            return ;
        }
        ParseError(err);
#else
        ParseError(strerror(errno));
#endif
}

void HCatUdpSocket::InitMember()
{
#ifdef H_WIN
    WORD ver = MAKEWORD(2, 2);
    WSAStartup(ver, &wsaData_);
#endif
}

void HCatUdpSocket::DestroyMember()
{
#ifdef H_WIN
    WSACleanup();
#endif
}

void HCatUdpSocket::DisposeEvent(hint32 fd)
{
    RallPoint<bool> threadRun(&threadRun_, false);
    if (threadExit_) {
        return ;
    }

    if (fd != this->socket_) {
        ParseError("parse read event error");
        return ;
    }

    struct sockaddr_in addr;
#ifdef H_WIN
    int addrLen = sizeof(struct sockaddr_in);
#else
    socklen_t addrLen = sizeof(struct sockaddr_in);
#endif
    char data[BUFF_MAX];

    hint32 size = recvfrom(this->socket_, data, BUFF_MAX, 0, reinterpret_cast<sockaddr *>(&addr), &addrLen);
    if (threadExit_) {
        return ;
    }

    if (size <= 0) {
        std::string log;
        NotifyError(&log);
        Close(true);
        ParseError(log);
        return ;
    }

    HCatBuffer readData(data, size);
    std::string ip(inet_ntoa(addr.sin_addr));
    huint16 port = ntohs(addr.sin_port);
    ICatEventApi::ForwardSignal(core_, this, ReadData, ip, port, readData);
}

void HCatUdpSocket::Close(bool updateSignal)
{
    this->RebirthID();
    if (this->socket_ == SOCKET_ERROR) {
        return ;
    }

#ifdef H_WIN
    closesocket(socket_);
#else
    close(socket_);
#endif

    this->socket_ = SOCKET_ERROR;

    if (updateSignal) {
        ICatEventApi::ForwardSignal(core_, this, Disconnected);
    }
}

void HCatUdpSocket::ParseSendData(const HCatBuffer &data)
{
    RallPoint<bool> threadRun(&threadRun_, false);
    if (threadExit_) {
        return ;
    }

    if (socket_ == SOCKET_ERROR) {
        ParseError("socket not open");
        return ;
    }

    sockaddr_in addrSrv;
    memset(&addrSrv, 0, sizeof(addrSrv));
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(port_);
    addrSrv.sin_addr.s_addr = sendIp_;

    hint32 size = sendto(socket_, data.ConstData(), data.Size(), 0, reinterpret_cast<sockaddr *>(&addrSrv), sizeof(addrSrv));
    if (threadExit_) {
        return ;
    }

    if (size == SOCKET_ERROR) {
        std::string log;
        this->NotifyError(&log);
        this->Close(true);
        this->ParseError(log);
    }
}

bool HCatUdpSocket::ParseConnectSignal()
{
    if (core_.expired()) {
        return false;
    }

    this->RebirthID();
    ICatEventApi::ForwardSignal(core_, this, Connected);
    ICatEventApi::AddIO(core_, socket_, this, &HCatUdpSocket::DisposeEvent, socket_);

    return true;
}

void HCatUdpSocket::ParseError(const std::string &error)
{
    ICatEventApi::ForwardSignal(core_, this, ErrorInfo, error);
}
