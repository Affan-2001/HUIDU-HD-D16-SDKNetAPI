

#ifndef __ICATEVENTBASE_H__
#define __ICATEVENTBASE_H__


#include <HCatEventStruct.h>


namespace cat
{




class HCATEVENT_EXPORT ICatEventBase
{
public:
    using H_EventPtr = std::shared_ptr<HCatEventUnit>;
    virtual ~ICatEventBase() {}

    ///< 将事件移动到对应事件队列去处理, 所有数据都以右值转发
    virtual void Move(H_EventPtr &&event) = 0;
};


using EventCorePtrType = std::shared_ptr<ICatEventBase>;
using EventCoreWeakPtr = std::weak_ptr<ICatEventBase>;

}


#endif // __ICATEVENTBASE_H__
