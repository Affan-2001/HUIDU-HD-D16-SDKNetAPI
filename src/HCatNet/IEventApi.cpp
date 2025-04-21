

#include <IEventApi.h>
#include <ICatEventApi.h>
#include <HCatSyncCoreEvent.h>
#include <HCatEventCore.h>


using namespace cat;


std::unique_ptr<IEventCore> IEventCore::instance_;



IEventCore::IEventCore()
    : core_(HCatEventCore::CreateCoreEvent())
{

}

HSlot IEventCore::PushSyncEvent(const std::function<void(const EventCoreWeakPtr &)> &func)
{
    return ICatEventApi::NotifyUpdateMainThread.Bind(func);
}

IEventCore::~IEventCore()
{

}

IEventCore *IEventCore::GetInstance()
{
    if (!instance_) {
        instance_.reset(new IEventCore());
    }

    return instance_.get();
}

EventCorePtrType IEventCore::CreateCoreEvnet()
{
    return HCatEventCore::CreateCoreEvent();
}

EventCorePtrType IEventCore::GetEventCore()
{
    return core_;
}

void IEventCore::MainThreadSync(const EventCoreWeakPtr &core, const std::function<void ()> &eventCall)
{
    ICatEventApi::MainThreadSync(core, eventCall);
}

void IEventCore::SwitchSyncThread(const EventCoreWeakPtr &core)
{
    ICatEventApi::SwitchMainThread(core);
}

bool IEventCore::IsNeedSyncEvent(const EventCoreWeakPtr &core)
{
    return ICatEventApi::IsNeedSyncEvent(core);
}
