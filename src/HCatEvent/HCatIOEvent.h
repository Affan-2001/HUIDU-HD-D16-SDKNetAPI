

#ifndef __HCATIOEVENT_H__
#define __HCATIOEVENT_H__


#include <ICatEventBase.h>
#include <HCatEventStruct.h>
#include <HCatThread.h>

#include <list>
#include <mutex>

#ifdef H_WIN
#include <WinSock2.h>
#else
#include <sys/time.h>
#include <unistd.h>
#include <poll.h>
#endif


namespace cat
{


class HCatIOEvent : public ICatEventBase
{
public:
    explicit HCatIOEvent(const EventCoreWeakPtr &core);
    ~HCatIOEvent();

    virtual void Move(H_EventPtr &&event) override;

private:
    void NotifyRun();
    void Push(H_EventPtr &&task);
    void Loop();
    void ParseTask();
    void Select(int maxFd, fd_set *readfds, fd_set *writefds, fd_set *excefds, timeval *timeout);

#ifndef H_WIN
    void ParsePollTask();
    ///< 和runList列表相关联
    void Poll(pollfd *fds, int num, int timeout);
#endif

private:
    EventCoreWeakPtr core_;
    std::list<H_EventPtr> taskList_;    ///< 事件任务列表
    std::list<H_EventPtr> runList_;     ///< 运行任务列表
    std::list<H_EventPtr> finishList_;  ///< 完成任务列表
    std::list<H_EventPtr> optList_;     ///< 加锁事件任务列表
    std::mutex lock_;
    std::unique_ptr<HCatThread> thread_;
    bool exit_;
};


}


#endif // __HCATIOEVENT_H__
