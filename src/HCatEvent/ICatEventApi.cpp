

#include <ICatEventApi.h>


using namespace cat;


HSignal<void(const EventCoreWeakPtr &)> ICatEventApi::NotifyUpdateMainThread;


void ICatEventApi::MainThreadSync(const EventCoreWeakPtr &core, const std::function<void ()> &eventCall)
{
    auto corePtr = core.lock();
    if (!corePtr) {
        return ;
    }

    HCatEventUnitPtr p = std::make_shared<HCatEventUnit>(kMainEvent);
    p->id = HCatEventId(p);
    p->optEvent = kRegister;
    p->callback = eventCall;
    corePtr->Move(std::move(p));
}

HCatAny ICatEventApi::QueryMainEvent(const EventCoreWeakPtr &core, HCatUpdateDataEvent::eUpdateEvent query)
{
    auto corePtr = core.lock();
    if (!corePtr) {
        return HCatAny();
    }

    HCatEventUnitPtr p = std::make_shared<HCatEventUnit>(kMainEvent);
    p->optEvent = kUpdateData;
    p->id = HCatEventId(p);
    HCatUpdateDataEvent isMain(query);
    p->data = isMain;
    corePtr->Move(std::move(p));
    return *isMain.data;
}

bool ICatEventApi::IsMainThread(const EventCoreWeakPtr &core)
{
    HCatAny result = QueryMainEvent(core, HCatUpdateDataEvent::kIsCreateThread);
    if (result.IsType<bool>() == false) {
        return false;
    }

    return result.Cast<bool>();
}

void ICatEventApi::SwitchMainThread(const EventCoreWeakPtr &core)
{
    QueryMainEvent(core, HCatUpdateDataEvent::kUpdateMainThread);
}

bool ICatEventApi::IsNeedSyncEvent(const EventCoreWeakPtr &core)
{
    HCatAny result = QueryMainEvent(core, HCatUpdateDataEvent::kIsNeedSyncEvent);
    if (result.IsType<bool>() == false) {
        return false;
    }

    return result.Cast<bool>();
}


void ICatEventApi::CreateEventSelectThread(const EventCoreWeakPtr &core, int event, int opt, bool runMainThread, std::function<void ()> &&func, HCatAny &&data)
{
    auto corePtr = core.lock();
    if (!corePtr) {
        return ;
    }

    HCatEventUnitPtr p = std::make_shared<HCatEventUnit>(event);
    p->optEvent = opt;
    p->id = HCatEventId(p);
    p->callRun = runMainThread ? HCatEventUnit::kMain : HCatEventUnit::kThread;
    p->callback = std::forward<std::function<void ()>>(func);
    p->data = std::forward<HCatAny>(data);
    corePtr->Move(std::move(p));
}

void ICatEventApi::CreateEventSelectThread(const EventCoreWeakPtr &core, int event, int opt, bool runMainThread, std::function<void()> &&func, HCatAny &&data, HCatEventUnitPtr &&obj)
{
    auto corePtr = core.lock();
    if (!corePtr || !obj) {
        return ;
    }

    HCatEventUnitPtr p = std::make_shared<HCatEventUnit>(event);
    p->optEvent = opt;
    p->id = HCatEventId(obj);
    p->callRun = runMainThread ? HCatEventUnit::kMain : HCatEventUnit::kThread;
    p->callback = std::forward<std::function<void ()>>(func);
    p->data = std::forward<HCatAny>(data);
    corePtr->Move(std::move(p));
}

void ICatEventApi::CreateEventFunc(const EventCoreWeakPtr &core, int event, int opt, std::function<void ()> &&func)
{
    auto corePtr = core.lock();
    if (!corePtr) {
        return ;
    }

    HCatEventUnitPtr p = std::make_shared<HCatEventUnit>(event);
    p->optEvent = opt;
    p->id = HCatEventId(p);
    p->callback = std::forward<std::function<void ()>>(func);
    corePtr->Move(std::move(p));
}

void ICatEventApi::CreateEventFunc(const EventCoreWeakPtr &core, int event, int opt, std::function<void ()> &&func, HCatEventUnitPtr &&obj)
{
    auto corePtr = core.lock();
    if (!corePtr || !obj) {
        return ;
    }

    HCatEventUnitPtr p = std::make_shared<HCatEventUnit>(event);
    p->optEvent = opt;
    p->id = HCatEventId(obj);
    p->callback = std::forward<std::function<void ()>>(func);
    corePtr->Move(std::move(p));
}

void ICatEventApi::CreateEventData(const EventCoreWeakPtr &core, int event, int opt, std::function<void ()> &&func, HCatAny &&data)
{
    auto corePtr = core.lock();
    if (!corePtr) {
        return ;
    }

    HCatEventUnitPtr p = std::make_shared<HCatEventUnit>(event);
    p->optEvent = opt;
    p->id = HCatEventId(p);
    p->callback = std::forward<std::function<void ()>>(func);
    p->data = std::forward<HCatAny>(data);
    corePtr->Move(std::move(p));
}

void ICatEventApi::CreateEventData(const EventCoreWeakPtr &core, int event, int opt, std::function<void ()> &&func, HCatAny &&data, HCatEventUnitPtr &&obj)
{
    auto corePtr = core.lock();
    if (!corePtr || !obj) {
        return ;
    }

    HCatEventUnitPtr p = std::make_shared<HCatEventUnit>(event);
    p->optEvent = opt;
    p->id = HCatEventId(obj);
    p->callback = std::forward<std::function<void ()>>(func);
    p->data = std::forward<HCatAny>(data);
    corePtr->Move(std::move(p));
}
