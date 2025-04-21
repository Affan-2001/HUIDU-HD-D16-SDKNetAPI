

#include <HCatEventCore.h>
#include <HCatEventStruct.h>
#include <HCatThread.h>
#include <HCatTimeEvent.h>
#include <HCatIOEvent.h>
#include <HCatMainEvent.h>
#include <HCatThreadTaskEvent.h>
#include <HCatIOReadWriteEvent.h>
#include <HCatGcEvent.h>


using namespace cat;


HCatEventCore::HCatEventCore()
{

}

EventCorePtrType HCatEventCore::CreateCoreEvent()
{
    auto core = std::shared_ptr<HCatEventCore>(new HCatEventCore());
    core->InitEvent();
    return core;
}

void HCatEventCore::Move(H_EventPtr &&event)
{
    const int type = event->eventType;
    if (type <= kMin || type > kMax) {
        return ;
    }

    const int opt = event->optEvent;
    switch (opt) {
    case kCallEnd:
    case kUpdateData:
    case kRegisterNotCall:
    case kRegister: {
        event_.at(type)->Move(std::forward<H_EventPtr>(event));
    } break;
    case kCall: {
        this->ParseCall(std::forward<H_EventPtr>(event));
    } break;
    case kIOReadWriteCall: {
        event_.at(kIOReadWriteEvent)->Move(std::forward<H_EventPtr>(event));
    } break;
    default:
        break;
    }
}

void HCatEventCore::InitEvent()
{

#define CREATE_EVENT(event, obj) \
    event_.at(event).reset(new obj(this->shared_from_this()))

    CREATE_EVENT(kMainEvent, HCatMainEvent);
    CREATE_EVENT(kThreadTaskEvent, HCatThreadTaskEvent);
    CREATE_EVENT(kTimeEvent, HCatTimeEvent);
    CREATE_EVENT(kIOEvent, HCatIOEvent);
    CREATE_EVENT(kIOReadWriteEvent, HCatIOReadWriteEvent);
    CREATE_EVENT(kGcEvent, HCatGcEvent);

#undef CREATE_EVENT
}

void HCatEventCore::ParseCall(H_EventPtr &&event)
{
    const int type = event->eventType;
    switch (event->callRun) {
    case HCatEventUnit::kThread:
        event_.at(kThreadTaskEvent)->Move(std::forward<H_EventPtr>(event));
        break;
    case HCatEventUnit::kMain:
        event_.at(kMainEvent)->Move(std::forward<H_EventPtr>(event));
        break;
    case HCatEventUnit::kCall:
        event_.at(type)->Move(std::forward<H_EventPtr>(event));
        break;
    default:
        break;
    }
}
