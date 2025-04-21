

#ifndef __HCATSYNCCOREEVENT_H__
#define __HCATSYNCCOREEVENT_H__


#include <IEventApi.h>
#include <HCatSignal.h>

#include <functional>
#include <condition_variable>
#include <mutex>


namespace cat
{


class HCatThread;

/**
 * @brief The HCatSyncCoreEvent class 事件核心同步
 */
class HCATNET_EXPORT HCatSyncCoreEvent
{
public:
    explicit HCatSyncCoreEvent(const std::function<void()> &eventCall = std::function<void()>());
    explicit HCatSyncCoreEvent(const EventCoreWeakPtr &core, const std::function<void()> &eventCall = std::function<void()>());
    ~HCatSyncCoreEvent();

    ///< 阻塞式运行事件核心
    bool Exec();

    ///< 开辟个线程运行, 非阻塞式
    bool Run();

    ///< 退出事件
    void Quit();

    ///< 获取事件核心
    const EventCorePtrType &GetEventCore() const { return core_; }

private:
    void NotifyUpdate(const EventCoreWeakPtr &syncCore);

private:
    EventCorePtrType core_;
    HSlot slot_;
    std::function<void()> eventCall_;
    std::condition_variable wait_;
    std::mutex lock_;
    std::unique_ptr<HCatThread> thread_;
    bool update_;
    bool exit_;
    bool run_;
};


}


#endif // HCATSYNCCOREEVENT_H
