

#include <HCatNet/HCatTcpService.h>
#include <ICatEventApi.h>
#include <HCatTool.h>

#include <cstring>

#ifdef H_WIN
#include <WinSock2.h>
#include <windows.h>

using sockLenType = int;

#pragma comment(lib,"ws2_32.lib")
#else
#include <sys/time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>


using sockLenType = socklen_t;

#define SOCKET_ERROR (-1)
#endif


using namespace cat;


HCatTcpService::HCatTcpService(const EventCoreWeakPtr &core)
    : core_(core)
    , socket_(SOCKET_ERROR)
    , threadExit_(false)
    , threadRun_(false)
{
    this->InitMember();
}

HCatTcpService::~HCatTcpService()
{
    threadExit_ = true;
    this->DestroyMember();
    while (threadRun_ && !core_.expired()) {
        threadExit_ = true;
        std::this_thread::yield();
    }
}

bool HCatTcpService::Listen(huint16 port)
{
    if (core_.expired()) {
        this->ParseError("Event core is nullptr");
        return false;
    }

    this->Close(false);

    socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (SOCKET_ERROR == socket_){
        return false;
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

    struct sockaddr_in addrSrv;
    memset(&addrSrv, 0, sizeof(addrSrv));
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(port);
    addrSrv.sin_addr.s_addr = htonl(INADDR_ANY);

    if (SOCKET_ERROR == bind(socket_, (sockaddr *)&addrSrv, sizeof(addrSrv))) {
        std::string log;
        this->NotifyError(&log);
        this->Close();
        ParseError(log);
        return false;
    }

    if (SOCKET_ERROR == listen(socket_, 1024)) {
        std::string log;
        this->NotifyError(&log);
        this->Close();
        ParseError(log);
        return false;
    }

    this->RebirthID();
    ICatEventApi::AddIO(core_, socket_, this, &HCatTcpService::DisposeEvent, socket_);

    Connected();
    return true;
}

bool HCatTcpService::IsListen() const
{
    return this->socket_ != SOCKET_ERROR;
}

void HCatTcpService::Close(bool signal)
{
    this->RebirthID();
    if (socket_ == SOCKET_ERROR) {
        return ;
    }
#ifdef H_WIN
    closesocket(socket_);
#else
    close(socket_);
#endif
    this->socket_ = SOCKET_ERROR;
    if (signal) {
        ICatEventApi::ForwardSignal(core_, this, Disconnected);
    }
}

void HCatTcpService::InitMember()
{
#ifdef H_WIN
    WORD ver = MAKEWORD(2, 2);
    WSAStartup(ver, &wsaData_);
#endif
}

void HCatTcpService::DestroyMember()
{
    this->Close();
#ifdef H_WIN
    WSACleanup();
#endif
}

void HCatTcpService::DisposeEvent(int fd)
{
    RallPoint<bool> threadRun(&threadRun_, false);
    if (threadExit_) {
        return ;
    }

    if (this->socket_ != fd) {
        return ;
    }

    struct sockaddr_in addrClient;
    sockLenType len = sizeof(addrClient);
    int client = accept(socket_, (struct sockaddr *)&addrClient, &len);
    if (threadExit_) {
        return ;
    }

    if (client == SOCKET_ERROR) {
        std::string log;
        this->NotifyError(&log);
        this->Close();
        this->ParseError(log);
        return ;
    }

    std::string ip = inet_ntoa(addrClient.sin_addr);
    huint16 port = ntohs(addrClient.sin_port);
    ICatEventApi::Forward(core_, this, &HCatTcpService::ParseNewConnect, client, ip, port);
}

void HCatTcpService::NotifyError(std::string *log)
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

void HCatTcpService::ParseError(const std::string &error)
{
    ICatEventApi::ForwardSignal(core_, this, ErrorInfo, error);
}

void HCatTcpService::ParseNewConnect(int fd, const std::string &ip, huint16 port)
{
    auto socketClient = std::make_shared<HCatTcpSocket>(core_, fd, ip, port);
    NewConnect(socketClient);
    socketClient->Connect();
}
