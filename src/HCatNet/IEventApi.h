

#ifndef __IEVENTAPI_H__
#define __IEVENTAPI_H__


#include <ISession.h>

#include <HCatNetInfo.h>
#include <HCatSignal.h>
#include <memory>
#include <mutex>




namespace cat
{


class ICatEventBase;
using EventCorePtrType = std::shared_ptr<ICatEventBase>;
using EventCoreWeakPtr = std::weak_ptr<ICatEventBase>;


/**
 * @brief The IEventCore class 同步事件的初始化需要构建一次这个接口单例
 */
class HCATNET_EXPORT IEventCore
{
public:
    ~IEventCore();
    static IEventCore *GetInstance();

    ///< 创建一个事件核心
    static EventCorePtrType CreateCoreEvnet();

    ///< 获取单例的事件核心
    EventCorePtrType GetEventCore();

    ///< 同步进程调用这个进行同步事件任务调度, core是需要调度事件核心
    ///< 非同步进程调用只会调用传入的eventCall, 不会同步事件任务
    ///< 在同步事件任务时可能会同步大量任务, 增加自身事件回调让它每次调度任务都会调用一次事件回调
    static void MainThreadSync(const EventCoreWeakPtr &core, const std::function<void()> &eventCall = [](){});

    ///< 切换当前线程为同步进程
    ///< 如果事件核心是不同线程创建的, 其他线程调用手动同步事件是无效的
    ///< 如需要其他线程来同步需要调用告知当前线程为同步进程
    static void SwitchSyncThread(const EventCoreWeakPtr &core);

    ///< 检查是否需要同步事件, 因通知同步事件只会通知一次, 在绑定事件后需检查是否有任务同步, 或者在事件线程先同步一次
    static bool IsNeedSyncEvent(const EventCoreWeakPtr &core);

    ///< 绑定通知同步事件函数
    static HSlot BindSyncEvent(const std::function<void(const EventCoreWeakPtr &)> &func) { return PushSyncEvent(func); }

    /**
     * @brief BindSyncEvent 绑定通知同步事件函数
     * @param obj 成员对象指针
     * @param func 成员函数, 类型是 void(const EventCoreWeakPtr &)
     * @return 返回绑定槽, 析构则自动断开绑定连接
     */
    template <typename _Obj, typename _ObjFunc>
    static HSlot BindSyncEvent(_Obj *obj, _ObjFunc func) { return PushSyncEvent(BindMember(obj, func)); }

private:
    IEventCore();
    static HSlot PushSyncEvent(const std::function<void (const EventCoreWeakPtr &)> &func);

private:
    static std::unique_ptr<IEventCore> instance_;
    EventCorePtrType core_;
};


}


#define CatEventCore() \
    cat::IEventCore::GetInstance()->GetEventCore()


#endif // __IEVENTAPI_H__
