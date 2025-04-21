

#ifndef __HCATSERIAL_H__
#define __HCATSERIAL_H__


#include <HCatEventCore.h>
#include <ICatEventData.h>
#include <HCatBuffer.h>
#include <HCatSignal.h>


#ifdef H_WIN
#include <windows.h>
#include <HCatTimer.h>
#include <HCatSignalSet.h>
#endif


namespace cat
{


class HCATEVENT_EXPORT HCatSerial : public HCatEventObject
{
private:
#ifdef H_WIN
    using fdType = HANDLE;
#else
    using fdType = int;
#endif

public:
    ///< 数据接收完的时候将发送该信号
    HSignal<void(const HCatBuffer &data)> ReadData;

    HSignal<void()> Connected;
    ///< 断开连接将发送该信号
    HSignal<void()> Disconnected;

    ///< 连接错误信息将发送该信号
    HSignal<void(const std::string &errorInfo)> ErrorInfo;

public:
    explicit HCatSerial(const EventCoreWeakPtr &core);
    ~HCatSerial();

    bool Open(const std::string &com, eBaudRate baud = kBaud9600, eDataBits bit = kData8, eParity parity = kNoParity, eStopBits stop = kOneStop);

    bool IsOpen() const;
    void Close(bool signal = true);

    bool SendData(const HCatBuffer &data);

private:
    void DisposeEvent(fdType fd);
    void ParseSendData(const HCatBuffer &data);

#ifdef H_WIN
    void DisposeCheck();
    bool ParseOpen(const std::string &com, eBaudRate baud, eDataBits bit, eParity parity, eStopBits stop);
#else
    bool ParseOpen(const std::string &com, eBaudRate baud, eDataBits bit, eParity parity, eStopBits stop);
#endif
    void ParseError(const std::string &error);
    void NotifyError(std::string *log = nullptr);


private:
    EventCoreWeakPtr core_;

#ifdef H_WIN
    HCatSignalSet slotSet_;
    HCatTimer timer_;
#endif

    fdType fd_;
    bool threadExit_;
    bool threadRun_;
};


}



#endif // HCATSERIAL_H
