

#include <HCApiImpl.h>
#include <ICatEventApi.h>


using namespace cat;


std::unique_ptr<HCApiImpl> HCApiImpl::instance_;

HCApiImpl *HCApiImpl::GetInstance()
{
    if (!instance_) {
        instance_.reset(new HCApiImpl());
    }

    return instance_.get();
}

HCApiImpl::~HCApiImpl()
{
    netList_.clear();
    syncCoreList_.clear();
    coreList_.clear();
}

ICatEventBase *HCApiImpl::CreateEventCore()
{
    std::unique_lock<std::mutex> lock(lock_);
    coreList_.emplace_back(IEventCore::CreateCoreEvnet());
    return coreList_.back().get();
}

void HCApiImpl::FreeEventCore(const ICatEventBase *core)
{
    if (!core) {
        return ;
    }

    FreeSyncEventCore(core);
    std::unique_lock<std::mutex> lock(lock_);
    for (auto i = coreList_.begin(); i != coreList_.end(); ++i) {
        if (i->get() != core) {
            continue;
        }

        i = coreList_.erase(i);
        return ;
    }
}

HCApiImpl::syncCorePtr HCApiImpl::CreateSyncEventCore(const ICatEventBase *core)
{
    if (!core) {
        return syncCorePtr();
    }

    std::unique_lock<std::mutex> lock(lock_);
    auto corePtr = FindEventCore(core);
    for (const auto &i : syncCoreList_) {
        if (i->GetEventCore() != corePtr) {
            continue;
        }

        return i;
    }

    syncCoreList_.emplace_back(syncCorePtr(new HCatSyncCoreEvent(corePtr)));
    return syncCoreList_.back();
}

void HCApiImpl::FreeSyncEventCore(const ICatEventBase *core)
{
    if (!core) {
        return ;
    }

    std::unique_lock<std::mutex> lock(lock_);
    auto corePtr = FindEventCore(core);
    for (auto i = syncCoreList_.begin(); i != syncCoreList_.end(); ++i) {
        if (i->get()->GetEventCore() != corePtr) {
            continue;
        }

        i->get()->Quit();
        i = syncCoreList_.erase(i);
        return ;
    }
}

ISession *HCApiImpl::CreateNetSession(const ICatEventBase *core)
{
    if (!core) {
        return nullptr;
    }

    std::unique_lock<std::mutex> lock(lock_);
    auto corePtr = FindEventCore(core);
    if (!corePtr) {
        return nullptr;
    }

    netList_.emplace_back(netPtr(new ISession(corePtr)));
    return netList_.back().get();
}

void HCApiImpl::AppendNetSession(const netPtr &net)
{
    if (!net) {
        return ;
    }

    std::unique_lock<std::mutex> lock(lock_);
    netList_.push_back(net);
}

void HCApiImpl::FreeNetSession(const ISession *net)
{
    if (!net) {
        return ;
    }

    std::unique_lock<std::mutex> lock(lock_);
    for (auto i = netList_.begin(); i != netList_.end(); ++i) {
        if (i->get() != net) {
            continue;
        }

        ICatEventApi::Gc(net->GetEventCore(), std::move(*i));
        i = netList_.erase(i);
        return ;
    }
}

HCApiImpl::HCApiImpl()
{

}


EventCorePtrType HCApiImpl::FindEventCore(const ICatEventBase *core)
{
    for (const auto &i : coreList_) {
        if (i.get() != core) {
            continue;
        }

        return i;
    }

    return EventCorePtrType();
}
