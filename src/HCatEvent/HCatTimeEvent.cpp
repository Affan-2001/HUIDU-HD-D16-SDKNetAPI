

#include <HCatTimeEvent.h>


using namespace cat;


HCatTimeEvent::HCatTimeEvent(const EventCoreWeakPtr &core)
    : core_(core)
    , thread_(new HCatThread())
    , exit_(false)
{

}

HCatTimeEvent::~HCatTimeEvent()
{
    this->exit_ = true;
    this->thread_.reset();
}

void HCatTimeEvent::Move(H_EventPtr &&event)
{
    // 核心调度器未注册
    if (core_.expired()) {
        return ;
    }

    // 失活
    if (event->id.IsInvalid()) {
        return ;
    }

    // 事件非法
    if (event->eventType != kTimeEvent) {
        return ;
    }

    switch (event->optEvent) {
    case kRegister: {
        Push(std::forward<H_EventPtr>(event));
    } break;
    case kCallEnd: {
        if (event->data.IsType<H_TimeType>() == false) {
            return ;
        }

        // 单次的定时器
        if (event->data.Cast<H_TimeType>().loop == false) {
            return ;
        }

        hint64 &value = event->data.Cast<H_TimeType>().currTime;
        value = std::chrono::duration_cast<U_UnitType>(U_ClockType::now().time_since_epoch()).count() - value;
        Push(std::forward<H_EventPtr>(event));
    } break;
    default:
        break;
    }

    return ;
}

void HCatTimeEvent::NotifyRun()
{
    if (thread_->IsRun() && thread_->Size() > 1) {
        return ;
    }

    thread_->Run(this, &HCatTimeEvent::Loop);
}


void HCatTimeEvent::Push(H_EventPtr &&task)
{
    std::unique_lock<std::mutex> lock(lock_);
    if (taskList_.empty() && thread_->IsRun() == false) {
        startTime_ = U_ClockType::now();
    }

    optList_.emplace_back(std::forward<H_EventPtr>(task));
    this->NotifyRun();
}

void HCatTimeEvent::Loop()
{
    for (;;) {
        if (exit_) {
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

        this->ParseTask();
    }
}


void HCatTimeEvent::ParseTask()
{
    auto calcMicroSec = [](const U_TimeType &start, const U_TimeType &end) {
        return std::chrono::duration_cast<U_UnitType>(end - start).count();
    };

    U_TimeType timeEnd = U_ClockType::now();
    const hint64 ms = calcMicroSec(startTime_, timeEnd);
    const hint64 schedulingTime = ms / 2;
    hint64 delay = 10000;

    if (core_.expired()) {
        taskList_.clear();
        return ;
    }

    for (auto i = taskList_.begin(); i != taskList_.end();) {
        if (exit_ || core_.expired()) {
            return ;
        }

        H_EventPtr p = *i;
        // 生存期过期的剔除
        if (p->id.IsInvalid()) {
            i = taskList_.erase(i);
            continue;
        }

        // 非法数据
        if (p->data.IsType<H_TimeType>() == false) {
            i = taskList_.erase(i);
            continue;
        }

        // 时间未到
        H_TimeType &data = p->data.Cast<H_TimeType>();
        data.currTime += ms;
        if (data.currTime + schedulingTime < data.targetTime) {
            hint64 diff = data.targetTime - data.currTime - schedulingTime;
            if (diff < delay) {
                delay = diff / 2;
            }
            ++i;
            continue;
        }

        // 转发到调度器进行调用操作
        i = taskList_.erase(i);
        p->optEvent = kCall;
        if (data.loop) {
            data.currTime = std::chrono::duration_cast<U_UnitType>(timeEnd.time_since_epoch()).count() - data.currTime + data.targetTime;
        }

        auto core = core_.lock();
        if (!core) {
            return ;
        }
        core->Move(std::move(p));
    }

    startTime_ = timeEnd;
    if (taskList_.empty() == false && exit_ == false) {
        std::this_thread::sleep_for(std::chrono::microseconds(delay));
    }
}
