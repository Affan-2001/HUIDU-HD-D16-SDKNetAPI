

#ifndef __HCATTHREADTASKEVENT_H__
#define __HCATTHREADTASKEVENT_H__



#include <ICatEventBase.h>
#include <HCatThread.h>

#include <list>
#include <mutex>



namespace cat
{



class HCatThreadTaskEvent : public ICatEventBase
{
public:
    explicit HCatThreadTaskEvent(const EventCoreWeakPtr &core);
    ~HCatThreadTaskEvent();

    virtual void Move(H_EventPtr &&event) override;

private:
    void ParseTask();

private:
    static const uint32_t MAX_THREAD_NUM;
    EventCoreWeakPtr core_;
    std::unique_ptr<HCatThreadPool> threadPool_;
    std::mutex lock_;
    std::list<H_EventPtr> taskList_;    ///< 事件任务列表
    bool exit_;
};


}


#endif // __HCATTHREADTASKEVENT_H__
