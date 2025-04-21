

#include <HCatGcEvent.h>


using namespace cat;


HCatGcEvent::HCatGcEvent(const EventCoreWeakPtr &core)
    : core_(core)
{

}

void HCatGcEvent::Move(H_EventPtr &&event)
{
    if (core_.expired()) {
        return ;
    }

    switch (event->optEvent) {
    case kRegister:
    case kRegisterNotCall: {
        this->Push(std::move(event));
        HCatEventUnitPtr notify = std::make_shared<HCatEventUnit>(kMainEvent, kRegisterNotCall);
        auto core = core_.lock();
        if (core) {
            core->Move(std::move(notify));
        }
    } break;
    case kCall: {
        this->ParseTask();
    } break;
    default:
        break;
    }
}

void HCatGcEvent::Push(H_EventPtr &&task)
{
    std::unique_lock<std::mutex> lock(lock_);
    optList_.emplace_back(std::move(task));
}

void HCatGcEvent::ParseTask()
{
    std::list<H_EventPtr> opt;
    {
        std::unique_lock<std::mutex> lock(lock_);
        opt.swap(optList_);
    }

    if (opt.empty()) {
        return ;
    }

    for (auto &&i : opt) {
        if (i->optEvent == kRegisterNotCall) {
            continue;
        }

        if (i->id.IsInvalid()) {
            continue;
        }

        if (i->callback) {
            i->callback();
        }
    }
}
