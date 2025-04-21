

#include <HCatNet/HCatSerial.h>
#include <ICatEventApi.h>
#include <HCatTool.h>

#ifdef H_WIN

#else
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#endif

#ifdef H_WIN
#define SERIAL_INIT_VALUE INVALID_HANDLE_VALUE
#define CloseSerial(fd) CloseHandle(fd)
#else
#define SERIAL_INIT_VALUE -1
#define CloseSerial(fd) close(fd)
#endif

#define BUFF_SIZE (1024)


using namespace cat;


HCatSerial::HCatSerial(const EventCoreWeakPtr &core)
    : core_(core)
    , fd_(SERIAL_INIT_VALUE)
    , threadExit_(false)
    , threadRun_(false)
{
#ifdef H_WIN
    slotSet_ += timer_.Timeout.Bind([this](){
        if (this->IsOpen() == false) {
            return ;
        }

        threadRun_ = !core_.expired();
        ICatEventApi::AddIOReadWrite(core_, this, &HCatSerial::DisposeCheck);
    });
#endif
}

HCatSerial::~HCatSerial()
{
    threadExit_ = true;
    this->Close(false);
    while (threadRun_ && !core_.expired()) {
        threadExit_ = true;
        std::this_thread::yield();
    }
}

bool HCatSerial::Open(const std::string &com, eBaudRate baud, eDataBits bit, eParity parity, eStopBits stop)
{
    if (this->IsOpen()) {
        this->Close();
    }

    return this->ParseOpen(com, baud, bit, parity, stop);
}

bool HCatSerial::IsOpen() const
{
    return this->fd_ != SERIAL_INIT_VALUE;
}

void HCatSerial::Close(bool signal)
{
#ifdef H_WIN
    timer_.Stop();
#else
    this->RebirthID();
#endif
    if (this->IsOpen() == false) {
        return ;
    }

    CloseSerial(fd_);
    fd_ = SERIAL_INIT_VALUE;

    if (signal) {
        ICatEventApi::ForwardSignal(core_, this, Disconnected);
    }
}

bool HCatSerial::SendData(const HCatBuffer &data)
{
    if (this->IsOpen() == false) {
        return false;
    }

    threadRun_ = !core_.expired();
    ICatEventApi::AddIOReadWrite(core_, this, &HCatSerial::ParseSendData, data);
    return true;
}

void HCatSerial::DisposeEvent(fdType fd)
{
#ifndef H_WIN
    RallPoint<bool> threadRun(&threadRun_, false);
#endif
    if (threadExit_) {
        return ;
    }

    if (fd != fd_ || this->IsOpen() == false) {
        return ;
    }

    char buff[BUFF_SIZE];
#ifdef H_WIN
    DWORD count = BUFF_SIZE;
    BOOL result = ReadFile(fd_, buff, count, &count, NULL);
    PurgeComm(fd_, PURGE_RXCLEAR | PURGE_RXABORT);
    if (!result) {
        return ;
    }
#else
    int count = read(fd_, buff, sizeof(buff));
    if (threadExit_) {
        return ;
    }

    if (count <= 0) {
        return ;
    }
#endif
    HCatBuffer readData(buff, count);
    ICatEventApi::ForwardSignal(core_, this, ReadData, readData);
}


void HCatSerial::ParseSendData(const HCatBuffer &data)
{
    RallPoint<bool> threadRun(&threadRun_, false);
    if (threadExit_) {
        return ;
    }

    if (this->IsOpen() == false) {
        return ;
    }

#ifdef H_WIN
    PurgeComm(fd_, PURGE_RXCLEAR | PURGE_TXCLEAR);
    DWORD writeSize = 0;
    WriteFile(fd_, data.ConstData(), data.Size(), &writeSize, NULL);
    PurgeComm(fd_, PURGE_RXCLEAR | PURGE_TXCLEAR);
#else
    write(fd_, data.ConstData(), data.Size());
#endif
}


#ifdef H_WIN

void HCatSerial::DisposeCheck()
{
    RallPoint<bool> threadRun(&threadRun_, false);
    if (threadExit_) {
        return ;
    }

    if (this->IsOpen() == false) {
        return ;
    }

    DWORD dwError = 0;
    //COMSTAT结构体,记录通信设备的状态信息
    COMSTAT  comstat;
    memset(&comstat, 0, sizeof(COMSTAT));
    // 清除以前遗留的错误标志
    if (ClearCommError(fd_, &dwError, &comstat) == false){
        return ;
    }

    if (comstat.cbInQue) {
        this->DisposeEvent(fd_);
    }
}

bool HCatSerial::ParseOpen(const std::string &com, eBaudRate baud, eDataBits bit, eParity parity, eStopBits stop)
{
    std::string openCom(com);
    // COM10以上需要特殊处理
    if (openCom.size() > 4) {
        openCom = R"(\\.\)" + com;
    }
    this->fd_ = CreateFileA(openCom.data(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (this->fd_ == SERIAL_INIT_VALUE) {
        std::string log;
        NotifyError(&log);
        Close(false);
        ParseError(log);
        return false;
    }

    if (SetupComm(fd_, BUFF_SIZE, BUFF_SIZE) == false) {
        std::string log;
        NotifyError(&log);
        Close(false);
        ParseError(log);
        return false;
    }

    DCB config;
    GetCommState(fd_, &config) ;
    config.BaudRate = baud;
    config.ByteSize = bit;
    config.Parity = parity;
    config.StopBits = stop;

    if (SetCommState(fd_, &config) == false) {
        std::string log;
        NotifyError(&log);
        Close(false);
        ParseError(log);
        return false;
    }

    //设置串口读写时间
    COMMTIMEOUTS CommTimeOuts;
    GetCommTimeouts(fd_, &CommTimeOuts);
    CommTimeOuts.ReadIntervalTimeout = MAXDWORD;
    CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
    CommTimeOuts.ReadTotalTimeoutConstant = 0;
    CommTimeOuts.WriteTotalTimeoutMultiplier = 10;
    CommTimeOuts.WriteTotalTimeoutConstant = 1000;
    if(!SetCommTimeouts(fd_, &CommTimeOuts)){
        std::string log;
        NotifyError(&log);
        Close(false);
        ParseError(log);
        return false;
    }

    Connected();
    timer_.Start(core_, 50);

    return true;
}

#else

bool HCatSerial::ParseOpen(const std::string &com, eBaudRate baud, eDataBits bit, eParity parity, eStopBits stop)
{
    this->fd_ = open(com.data(), O_RDWR | O_NOCTTY);
    if (this->fd_ == SERIAL_INIT_VALUE) {
        return false;
    }

    struct termios config;
    if (tcgetattr(this->fd_, &config) != 0) {
        this->Close();
        return false;
    }

    cfmakeraw(&config);
    config.c_cflag &= ~CSIZE;
    config.c_cflag |= CLOCAL | CREAD;

    int speed = 0;
    switch (baud) {
    case kBaud2400:
        speed = B2400;
        break;
    case kBaud4800:
        speed = B4800;
        break;
    case kBaud9600:
        speed = B9600;
        break;
    case kBaud19200:
        speed = B19200;
        break;
    case kBaud38400:
        speed = B38400;
        break;
    case kBaud115200:
        speed = B115200;
        break;
    default:
        speed = B9600;
        break;
    }

    cfsetispeed(&config, speed);
    cfsetospeed(&config, speed);

    switch (bit) {
    case kData5:
        config.c_cflag |= CS5;
        break;
    case kData6:
        config.c_cflag |= CS6;
        break;
    case kData7:
        config.c_cflag |= CS7;
        break;
    case kData8:
        config.c_cflag |= CS8;
        break;
    }

    switch (parity) {
    default:
    case kNoParity:
        config.c_cflag &= ~PARENB;
        config.c_iflag &= ~INPCK;
        break;
    case kOddParity:
        config.c_cflag |= (PARODD | PARENB);
        config.c_iflag |= INPCK;
        break;
    case kEvenParity:
        config.c_cflag |= PARENB;
        config.c_cflag &= ~PARODD;
        config.c_iflag |= INPCK;
        break;
    case kSpaceParity:
        config.c_cflag &= ~PARENB;
        config.c_cflag &= ~CSTOPB;
        break;
    }

    switch (stop) {
    default:
    case kOneStop:
        config.c_cflag &= ~CSTOPB;
        break;
    case kTwoStop:
        config.c_cflag |= CSTOPB;
        break;
    }

    config.c_cc[VTIME] = 0;
    config.c_cc[VMIN] = 1;
    tcflush(this->fd_, TCIFLUSH);
    if (tcsetattr(this->fd_, TCSANOW, &config) != 0) {
        this->Close();
        return false;
    }

    this->RebirthID();
    ICatEventApi::AddIO(core_, fd_, this, &HCatSerial::DisposeEvent, fd_);
    return true;
}
#endif

void HCatSerial::ParseError(const std::string &error)
{
    if (error.empty()) {
        return ;
    }

    ICatEventApi::ForwardSignal(core_, this, ErrorInfo, error);
}


void HCatSerial::NotifyError(std::string *log)
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
