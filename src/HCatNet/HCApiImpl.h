

#ifndef __HCAPIIMPL_H__
#define __HCAPIIMPL_H__


#include <IEventApi.h>
#include <HCatSyncCoreEvent.h>


namespace cat
{


class HCApiImpl
{
public:
    using netPtr = std::shared_ptr<ISession>;
    using syncCorePtr = std::shared_ptr<HCatSyncCoreEvent>;

public:
    static HCApiImpl *GetInstance();
    ~HCApiImpl();

    ICatEventBase *CreateEventCore();
    void FreeEventCore(const ICatEventBase *core);

    syncCorePtr CreateSyncEventCore(const ICatEventBase *core);
    void FreeSyncEventCore(const ICatEventBase *core);

    ISession *CreateNetSession(const ICatEventBase *core);
    void AppendNetSession(const netPtr &net);
    void FreeNetSession(const ISession *net);

private:
    HCApiImpl();
    EventCorePtrType FindEventCore(const ICatEventBase *core);

private:
    static std::unique_ptr<HCApiImpl> instance_;

    std::mutex lock_;
    std::list<EventCorePtrType> coreList_;
    std::list<syncCorePtr> syncCoreList_;
    std::list<netPtr> netList_;
};


}


#define CApiImpl() \
    cat::HCApiImpl::GetInstance()


#endif // __HCAPIIMPL_H__
