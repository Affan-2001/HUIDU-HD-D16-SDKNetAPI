

#include <HCatSyncCoreEvent.h>
#include <ICatEventApi.h>
#include <HCatThread.h>
#include <IEventApi.h>


using namespace cat;


HCatSyncCoreEvent::HCatSyncCoreEvent(const std::function<void ()> &eventCall)
    : HCatSyncCoreEvent(HCatEventCore::CreateCoreEvent(), eventCall)
{

}

HCatSyncCoreEvent::HCatSyncCoreEvent(const EventCoreWeakPtr &core, const std::function<void ()> &eventCall)
    : core_(core.lock())
    , eventCall_(eventCall)
    , update_(false)
    , exit_(false)
    , run_(false)
{
    slot_ = ICatEventApi::NotifyUpdateMainThread.Bind(this, &HCatSyncCoreEvent::NotifyUpdate);
}

HCatSyncCoreEvent::~HCatSyncCoreEvent()
{
    slot_.Clear();
    this->Quit();
}

bool HCatSyncCoreEvent::Exec()
{
    if (exit_) {
        return false;
    }

    run_ = true;
    ICatEventApi::SwitchMainThread(core_);
    update_ = ICatEventApi::IsNeedSyncEvent(core_);
    for (;;) {
        {
            std::unique_lock<std::mutex> lock(lock_);
            wait_.wait(lock, [this](){ return update_ || exit_; });
            update_ = false;
        }

        if (exit_) {
            break ;
        }

        ICatEventApi::MainThreadSync(core_, eventCall_);
    }

    exit_ = false;
    run_ = false;
    return true;
}

bool HCatSyncCoreEvent::Run()
{
    if (thread_ || run_) {
        return false;
    }

    thread_.reset(new HCatThread());
    thread_->Run(this, &HCatSyncCoreEvent::Exec);
    return true;
}

void HCatSyncCoreEvent::Quit()
{
    {
        std::unique_lock<std::mutex> lock(lock_);
        exit_ = true;
    }
    wait_.notify_all();
    thread_.reset();
}

void HCatSyncCoreEvent::NotifyUpdate(const EventCoreWeakPtr &syncCore)
{
    if (syncCore.lock() != core_) {
        return ;
    }

    {
        std::unique_lock<std::mutex> lock(lock_);
        update_ = true;
    }
    wait_.notify_all();
}
