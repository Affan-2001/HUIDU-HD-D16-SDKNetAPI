

#ifndef __HCATMAINEVENT_H__
#define __HCATMAINEVENT_H__


#include <ICatEventBase.h>

#include <list>
#include <mutex>
#include <thread>


namespace cat
{



class HCatMainEvent : public ICatEventBase
{
public:
    explicit HCatMainEvent(const EventCoreWeakPtr &core);
    ~HCatMainEvent();

    virtual void Move(H_EventPtr &&event) override;

private:
    void Push(H_EventPtr &&event);
    void ParseTask(H_EventPtr &&event);
    void ParseQueryEvent(H_EventPtr &&event);

private:
    EventCoreWeakPtr core_;
    std::list<H_EventPtr> taskList_;    ///< 事件任务列表
    std::mutex lock_;
    std::thread::id threadId_;
    bool sync_;
    bool run_;
};



}



#endif // HCATMAINEVENT_H
