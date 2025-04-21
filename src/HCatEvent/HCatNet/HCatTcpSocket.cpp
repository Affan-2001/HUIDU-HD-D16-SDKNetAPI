

#include <HCatNet/HCatTcpSocket.h>
#include <HCatTool.h>
#include <ICatEventApi.h>

#include <cstring>
#include <regex>

#ifdef H_WIN
#include <WinSock2.h>
#include <windows.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <sys/time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SOCKET_ERROR (-1)
#endif


#define BUFF_MAX    (1024 * 9)


using namespace cat;


HCatTcpSocket::HCatTcpSocket(const EventCoreWeakPtr &core)
    : core_(core)
    , socket_(SOCKET_ERROR)
    , port_(0)
    , threadExit_(false)
    , threadRun_(false)
{
    this->InitMember();
}

HCatTcpSocket::HCatTcpSocket(const EventCoreWeakPtr &core, int socket, const std::string &ip, huint16 port)
    : core_(core)
    , socket_(socket)
    , ip_(ip)
    , port_(port)
    , threadExit_(false)
    , threadRun_(false)
{
    this->InitMember();
}

HCatTcpSocket::~HCatTcpSocket()
{
    threadExit_ = true;
    this->DestroyMember();
    thread_.Quit();
    while (threadRun_ && !core_.expired()) {
        threadExit_ = true;
        std::this_thread::yield();
    }
}

bool HCatTcpSocket::Connect()
{
    if (SOCKET_ERROR == socket_) {
        return false;
    }

    return this->ParseConnectSignal();
}

bool HCatTcpSocket::Connect(const std::string &ip, huint16 port, bool threadRun)
{
    if (core_.expired()) {
        this->ParseError("Event core is nullptr");
        return false;
    }

    if (ip.empty() || IsVaildAddr(ip) == false) {
        ParseError("Invalid Address");
        return false;
    }

    this->Close(false);

    this->socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (SOCKET_ERROR == socket_){
        ParseError("create socket faild");
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
    addrSrv.sin_addr.s_addr = inet_addr(ip.data());

    if (threadRun) {
        thread_.Run(this, &HCatTcpSocket::ParseConnect, ip, port, addrSrv);
        return true;
    }

    this->ParseConnect(ip, port, addrSrv);
    return this->IsConnect();
}

void HCatTcpSocket::Disconnect(bool signal)
{
    this->Close(signal);
}

bool HCatTcpSocket::IsConnect() const
{
    return this->socket_ != SOCKET_ERROR;
}

bool HCatTcpSocket::SendData(const HCatBuffer &data)
{
    if (this->socket_ == SOCKET_ERROR) {
        return false;
    }

    threadRun_ = !core_.expired();
    ICatEventApi::AddIOReadWrite(core_, this, &HCatTcpSocket::ParseSendData, data);
    return this->IsConnect();
}

bool HCatTcpSocket::IsIpv4(const std::string &ip)
{
    std::smatch res;
    std::regex ipv4(R"(^((25[0-5]|2[0-4]\d|[01]?\d\d?)\.){3}(25[0-5]|2[0-4]\d|[01]?\d\d?)$)");
    std::string addr(ip.substr(0, ip.find(R"(:)")));
    return std::regex_match(addr, res, ipv4);
}

bool HCatTcpSocket::IsIpv6(const std::string &ip)
{
    std::smatch res;
    std::regex ipv6(R"(^([\da-fA-F]{1,4}:){7}[\da-fA-F]{1,4}$)");
    return std::regex_match(ip, res, ipv6);
}

std::string HCatTcpSocket::UrlToHost(const std::string &url)
{
    if (url.empty()) {
        return url;
    }

    std::size_t pos = url.find(R"(://)");

    // 检查是否存在标志头
    if (pos != std::string::npos) {
        pos += 3;
        return url.substr(pos, url.find(R"(/)", pos) - pos);
    }

    std::size_t lastPos = url.find(R"(:)");
    if (lastPos == std::string::npos) {
        return url.substr(0, url.find(R"(/)"));
    }

    return url.substr(0, lastPos);
}

std::string HCatTcpSocket::UrlToResource(const std::string &url)
{
    if (url.empty()) {
        return url;
    }

    std::size_t pos = url.find(R"(://)");

    // 检查是否存在标志头
    if (pos != std::string::npos) {
        pos += 3;
        return url.substr(url.find(R"(/)", pos));
    }

    pos = url.find(R"(/)");
    if (pos == std::string::npos) {
        return url;
    }

    return url.substr(pos);
}

std::list<std::string> HCatTcpSocket::UrlToIp(const std::string &url)
{
#ifdef H_WIN
    // 用于激活windows网络库初始化
    static auto tcpSocket = std::make_unique<HCatTcpSocket>(EventCoreWeakPtr());
#endif

    std::list<std::string> ipList;
    std::string hostAddr = UrlToHost(url);
    struct hostent *host = gethostbyname(hostAddr.data());
    if (!host) {
        return ipList;
    }

    switch (host->h_addrtype) {
    case AF_INET:
    case AF_INET6:
        break;
    default:
        return ipList;
        break;
    }

    for(int i = 0; host->h_addr_list[i]; ++i){
        struct in_addr *addr = reinterpret_cast<struct in_addr *>(host->h_addr_list[i]);
        ipList.emplace_back(inet_ntoa(*addr));
    }

    return ipList;
}

void HCatTcpSocket::InitMember()
{
#ifdef H_WIN
    WORD ver = MAKEWORD(2, 2);
    WSAStartup(ver, &wsaData_);
#endif
}

void HCatTcpSocket::DestroyMember()
{
    this->Close(false);
#ifdef H_WIN
    WSACleanup();
#endif
}

void HCatTcpSocket::Close(bool updateSignal)
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

void HCatTcpSocket::DisposeEvent(hint32 fd)
{
    RallPoint<bool> threadRun(&threadRun_, false);
    if (threadExit_) {
        return ;
    }

    if (fd != this->socket_) {
        ParseError("parse read event error");
        return ;
    }

    char data[BUFF_MAX];
    hint32 size = recv(this->socket_, data, BUFF_MAX, 0);
    if (threadExit_) {
        return ;
    }

    if (size <= 0) {
        std::string log;
        if (size < 0) {
            this->NotifyError(&log);
        }
        ICatEventApi::Forward(core_, this, &HCatTcpSocket::Close, true);
        ParseError(log);
        return ;
    }

    HCatBuffer readData(data, size);
    ICatEventApi::ForwardSignal(core_, this, ReadData, readData);
}

void HCatTcpSocket::ParseSendData(const HCatBuffer &data)
{
    RallPoint<bool> threadRun(&threadRun_, false);
    if (threadExit_) {
        return ;
    }

    if (socket_ == SOCKET_ERROR) {
        ParseError("socket not open");
        return ;
    }

    hint64 size = send(socket_, data.ConstData(), data.Size(), 0);
    if (threadExit_) {
        return ;
    }

    if (size != static_cast<hint64>(data.Size())) {
        std::string log;
        this->NotifyError(&log);
        this->Close(true);
        this->ParseError(log);
    }
}

void HCatTcpSocket::ParseConnect(const std::string &ip, huint16 port, const HCatAny &data)
{
    if (data.IsType<sockaddr_in>() == false) {
        ParseError("parse connect iconv type error");
        return ;
    }
    sockaddr_in addrSrv = data.Cast<sockaddr_in>();

    int result = connect(socket_, (struct sockaddr *)&addrSrv, sizeof(addrSrv));
    if (threadExit_) {
        return ;
    }

    if (result < 0) {
        if (socket_ != SOCKET_ERROR) {
            std::string log;
            this->NotifyError(&log);
            this->Close(!this->IsConnect());
            this->ParseError(log);
        }
        return ;
    }

    this->ip_ = ip;
    this->port_ = port;
    this->ParseConnectSignal();
}

bool HCatTcpSocket::ParseConnectSignal()
{
    if (core_.expired()) {
        return false;
    }

    this->RebirthID();
    ICatEventApi::ForwardSignal(core_, this, Connected);
    ICatEventApi::AddIO(core_, socket_, this, &HCatTcpSocket::DisposeEvent, socket_);

    return true;
}

void HCatTcpSocket::ParseError(const std::string &error)
{
    if (error.empty()) {
        return ;
    }

    ICatEventApi::ForwardSignal(core_, this, ErrorInfo, error);
}

void HCatTcpSocket::NotifyError(std::string *log)
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
