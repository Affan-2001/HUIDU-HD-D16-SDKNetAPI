


#include <HCatIOReadWriteEvent.h>


using namespace cat;


HCatIOReadWriteEvent::HCatIOReadWriteEvent(const EventCoreWeakPtr &core)
    : core_(core)
    , thread_(new HCatThread)
    , exit_(false)
{

}

HCatIOReadWriteEvent::~HCatIOReadWriteEvent()
{
    this->exit_ = true;
    thread_.reset();
}

void HCatIOReadWriteEvent::Move(H_EventPtr &&event)
{
    if (core_.expired()) {
        return ;
    }

    if (event->id.IsInvalid()) {
        return ;
    }

    switch (event->optEvent) {
    case kIOReadWriteCall:
    case kRegister: {
        Push(std::forward<H_EventPtr>(event));
    } break;
    default:
        break;
    }

    return ;
}

void HCatIOReadWriteEvent::NotifyRun()
{
    if (thread_->IsRun() && thread_->Size() > 1) {
        return ;
    }

    thread_->Run(this, &HCatIOReadWriteEvent::Loop);
}

void HCatIOReadWriteEvent::Push(H_EventPtr &&task)
{
    std::unique_lock<std::mutex> lock(lock_);
    optList_.emplace_back(std::forward<H_EventPtr>(task));
    this->NotifyRun();
}

void HCatIOReadWriteEvent::Loop()
{
    for (;;) {
        if (exit_ || core_.expired()) {
            return ;
        }

        // 直接和数据进行交换, 确保锁停留的时间最短
        std::list<H_EventPtr> opt;
        {
            std::unique_lock<std::mutex> lock(lock_);
            opt.swap(optList_);
        }

        if (opt.empty()) {
            return ;
        }

        this->ParseTask(std::move(opt));
    }
}

void HCatIOReadWriteEvent::ParseTask(std::list<H_EventPtr> &&task)
{
    for (auto &&i : task) {
        if (i->id.IsInvalid()) {
            continue;
        }

        if (i->callback) {
            i->callback();
        }
        i->optEvent = kCallEnd;

        auto core = core_.lock();
        if (!core) {
            return ;
        }
        core->Move(std::move(i));
    }
}
