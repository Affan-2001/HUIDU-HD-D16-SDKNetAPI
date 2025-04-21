

#ifndef __HCATGCEVENT_H__
#define __HCATGCEVENT_H__


#include <ICatEventBase.h>

#include <list>
#include <mutex>


namespace cat
{


class HCatGcEvent : public ICatEventBase
{
public:
    explicit HCatGcEvent(const EventCoreWeakPtr &core);

    virtual void Move(H_EventPtr &&event) override;

private:
    void Push(H_EventPtr &&task);
    void ParseTask();

private:
    EventCoreWeakPtr core_;
    std::list<H_EventPtr> optList_;
    std::mutex lock_;
};


}


#endif // __HCATGCEVENT_H__
