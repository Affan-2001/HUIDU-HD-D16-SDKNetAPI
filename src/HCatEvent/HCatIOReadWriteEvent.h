

#ifndef __HCATIOREADWRITEEVENT_H__
#define __HCATIOREADWRITEEVENT_H__



#include <ICatEventBase.h>
#include <HCatThread.h>

#include <list>
#include <mutex>


namespace cat
{



class HCatIOReadWriteEvent : public ICatEventBase
{
public:
    explicit HCatIOReadWriteEvent(const EventCoreWeakPtr &core);
    ~HCatIOReadWriteEvent();
    virtual void Move(H_EventPtr &&event) override;

private:
    void NotifyRun();
    void Push(H_EventPtr &&task);
    void Loop();
    void ParseTask(std::list<H_EventPtr> &&task);

private:
    EventCoreWeakPtr core_;
    std::list<H_EventPtr> optList_;     ///< 加锁事件任务列表
    std::mutex lock_;
    std::unique_ptr<HCatThread> thread_;
    bool exit_;
};


}


#endif // HCATIOREADWRITEEVENT_H
