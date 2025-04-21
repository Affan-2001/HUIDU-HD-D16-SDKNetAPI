

#include <HCatIOEvent.h>
#include <vector>

#ifdef H_WIN
#pragma comment(lib,"Ws2_32.lib")
#endif


#define IO_TIME     (0)
#define IO_UTIME    (1)
#define IO_MTIME    (1)


using namespace cat;


HCatIOEvent::HCatIOEvent(const EventCoreWeakPtr &core)
    : core_(core)
    , thread_(new HCatThread)
    , exit_(false)
{

}

HCatIOEvent::~HCatIOEvent()
{
    this->exit_ = true;
    this->thread_.reset();
}

void HCatIOEvent::Move(H_EventPtr &&event)
{
    if (core_.expired()) {
        return ;
    }

    // 失活
    if (event->id.IsInvalid()) {
        return ;
    }

    switch (event->optEvent) {
    case kRegister: {
        Push(std::forward<H_EventPtr>(event));
    } break;
    case kCallEnd: {
        if (event->data.IsType<H_IOType>() == false) {
            return ;
        }

        Push(std::forward<H_EventPtr>(event));
    } break;
    default:
        break;
    }

    return ;
}

void HCatIOEvent::NotifyRun()
{
    if (thread_->IsRun() && thread_->Size() > 1) {
        return ;
    }

    thread_->Run(this, &HCatIOEvent::Loop);
}

void HCatIOEvent::Push(H_EventPtr &&task)
{
    std::unique_lock<std::mutex> lock(lock_);
    optList_.emplace_back(std::forward<H_EventPtr>(task));
    this->NotifyRun();
}

void HCatIOEvent::Loop()
{
    for (;;) {
        if (exit_ || core_.expired()) {
            break;
        }

        // 直接和数据进行交换, 确保锁停留的时间最短
        std::list<H_EventPtr> opt;
        {
            std::unique_lock<std::mutex> lock(lock_);
            opt.swap(optList_);
        }

        if (opt.empty() == false) {
            taskList_.splice(taskList_.end(), std::move(opt));
        }

        if (taskList_.empty()) {
            break;
        }

#ifdef H_WIN
        this->ParseTask();
#else
        this->ParsePollTask();
#endif
    }
}

void HCatIOEvent::ParseTask()
{
    struct timeval timeout;
    timeout.tv_sec = IO_TIME;
    timeout.tv_usec = IO_UTIME;

    hint32 maxFd = -1;
    fd_set readfds;
    FD_ZERO(&readfds);
    int calcNum = 0;
    for (auto i = taskList_.begin(); i != taskList_.end();) {
        H_EventPtr p = std::move(*i);
        i = taskList_.erase(i);
        // 生存期过期的剔除
        if (p->id.IsInvalid()) {
            continue;
        }

        // 非法数据
        if (p->data.IsType<H_IOType>() == false) {
            continue;
        }

        H_IOType fd = p->data.Cast<H_IOType>();
        FD_SET(fd, &readfds);
        if (fd > maxFd) {
            maxFd = fd;
        }

        runList_.emplace_back(std::move(p));
        ++calcNum;
        if (calcNum + 1 >= FD_SETSIZE) {
            this->Select(maxFd, &readfds, nullptr, nullptr, &timeout);
            FD_ZERO(&readfds);
            calcNum = 0;
            timeout.tv_sec = IO_TIME;
            timeout.tv_usec = IO_UTIME;
            maxFd = -1;
        }
    }

    this->Select(maxFd, &readfds, nullptr, nullptr, &timeout);
    taskList_.splice(taskList_.end(), std::move(finishList_));
}

void HCatIOEvent::Select(int maxFd, fd_set *readfds, fd_set *writefds, fd_set *excefds, timeval *timeout)
{
    int ret = select(maxFd + 1, readfds, writefds, excefds, timeout);
    if (ret <= 0) {
        finishList_.splice(finishList_.end(), std::move(runList_));
        return ;
    }

    if (core_.expired()) {
        finishList_.splice(finishList_.end(), std::move(runList_));
        return ;
    }

    for (auto i = runList_.begin(); i != runList_.end();) {
        H_EventPtr p = std::move(*i);
        i = runList_.erase(i);
        // 生存期过期的剔除
        if (p->id.IsInvalid()) {
            continue;
        }

        H_IOType fd = p->data.Cast<H_IOType>();
        if (FD_ISSET(fd, readfds) == false) {
            finishList_.emplace_back(std::move(p));
            continue;
        }

        // 转发到调度器进行调用操作
        p->optEvent = kIOReadWriteCall;

        auto core = core_.lock();
        if (!core) {
            return ;
        }
        core->Move(std::move(p));
    }
}


#ifndef H_WIN
void HCatIOEvent::ParsePollTask()
{
    std::vector<pollfd> readfds;
    int timeout = IO_TIME * 1000 + IO_MTIME;
    int calcNum = 0;
    for (auto i = taskList_.begin(); i != taskList_.end();) {
        H_EventPtr p = std::move(*i);
        i = taskList_.erase(i);
        // 生存期过期的剔除
        if (p->id.IsInvalid()) {
            continue;
        }

        // 非法数据
        if (p->data.IsType<H_IOType>() == false) {
            continue;
        }

        pollfd pfd;
        pfd.fd = p->data.Cast<H_IOType>();
        pfd.events = POLLIN | POLLRDNORM;
        pfd.revents = 0;
        readfds.emplace_back(std::move(pfd));

        runList_.emplace_back(std::move(p));
        ++calcNum;
        if (calcNum + 1 >= FD_SETSIZE) {
            this->Poll(readfds.data(), readfds.size(), timeout);
            readfds.clear();
            calcNum = 0;
        }
    }

    this->Poll(readfds.data(), readfds.size(), timeout);
    taskList_.splice(taskList_.end(), std::move(finishList_));
}

void HCatIOEvent::Poll(pollfd *fds, int num, int timeout)
{
    if (num != static_cast<int>(runList_.size()) || runList_.empty()) {
        return ;
    }

    int ret = poll(fds, num, timeout);
    if (ret <= 0) {
        finishList_.splice(finishList_.end(), std::move(runList_));
        return ;
    }

    if (core_.expired()) {
        finishList_.splice(finishList_.end(), std::move(runList_));
        return ;
    }


    for (int i = 0; i < num; ++i) {
        if (runList_.empty()) {
            return ;
        }

        H_EventPtr p = std::move(runList_.front());
        runList_.pop_front();

        // 生存期过期的剔除
        if (p->id.IsInvalid()) {
            continue;
        }

        H_IOType fd = p->data.Cast<H_IOType>();
        if (fd != fds[i].fd) {
            finishList_.emplace_back(std::move(p));
            continue;
        }

        if (!(fds[i].revents & (POLLRDNORM | POLLERR | POLLIN))) {
            finishList_.emplace_back(std::move(p));
            continue;
        }

        // 转发到调度器进行调用操作
        p->optEvent = kIOReadWriteCall;

        auto core = core_.lock();
        if (!core) {
            return ;
        }
        core->Move(std::move(p));
    }
}
#endif
