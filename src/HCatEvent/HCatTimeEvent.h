

#ifndef __HCATTIMEEVENT_H__
#define __HCATTIMEEVENT_H__


#include <ICatEventBase.h>
#include <HCatEventStruct.h>
#include <HCatThread.h>

#include <mutex>
#include <chrono>
#include <list>


namespace cat
{


class HCatTimeEvent : public ICatEventBase
{
public:
    explicit HCatTimeEvent(const EventCoreWeakPtr &core);
    virtual ~HCatTimeEvent();

    virtual void Move(H_EventPtr &&event) override;

private:
    void NotifyRun();
    void Push(H_EventPtr &&task);
    void Loop();
    void ParseTask();

private:
    using U_ClockType = std::chrono::high_resolution_clock;
    using U_TimeType = U_ClockType::time_point;
    using U_UnitType = std::chrono::microseconds;

private:
    EventCoreWeakPtr core_;
    U_TimeType startTime_;
    std::list<H_EventPtr> taskList_;    ///< 事件任务列表
    std::list<H_EventPtr> optList_;     ///< 加锁事件任务列表
    std::mutex lock_;
    std::unique_ptr<HCatThread> thread_;
    bool exit_;
};


}


#endif // __HCATTIMEEVENT_H__
