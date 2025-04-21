

#ifndef __HCATEVENTCORE_H__
#define __HCATEVENTCORE_H__


#include <memory>
#include <mutex>
#include <list>
#include <array>
#include <thread>
#include <condition_variable>

#include <ICatEventBase.h>
#include <HCatEventInfo.h>


namespace cat
{


/**
 * @brief The HCatEventCore class 事件分配器
 * 所有的事件都像猫一样, 完成就跑到自己的地方
 * 核心只将事件转发到对应事件处理器, 自身不做事件修改
 */
class HCATEVENT_EXPORT HCatEventCore : public ICatEventBase, public std::enable_shared_from_this<HCatEventCore>
{
public:
    ///< 构建事件核心必须使用该方法构建
    static EventCorePtrType CreateCoreEvent();

    virtual void Move(H_EventPtr &&event) override;

private:
    HCatEventCore();
    void InitEvent();
    void ParseCall(H_EventPtr &&event);

private:
    std::array<std::unique_ptr<ICatEventBase>, kMax> event_;
};


}


#endif // __HCATEVENTCORE_H__
