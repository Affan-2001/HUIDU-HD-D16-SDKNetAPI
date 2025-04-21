

#ifndef HCATNET_LIBRARY

#ifndef __HCATEVENTTOQTEVENT_H__
#define __HCATEVENTTOQTEVENT_H__


#include <HCatEventInfo.h>
#include <IEventApi.h>

#include <QObject>
#include <QCoreApplication>
#include <QTimer>


namespace cat
{


/**
 * @brief The HEventToQtEvent class 将事件机制转换到Qt事件
 */
class HEventToQtEvent : public QObject
{
    Q_OBJECT
public:
    explicit HEventToQtEvent(const EventCoreWeakPtr &core)
        : core_(core)
    {
        // 切换到Qt主进程同步事件
        QTimer::singleShot(0, this, [this](){
            IEventCore::SwitchSyncThread(core_);
            emit Notify();
        });

        notifyUpdateSlot_ = IEventCore::BindSyncEvent(this, &HEventToQtEvent::Wall);
        connect(this, &HEventToQtEvent::Notify, this, [this](){
            IEventCore::MainThreadSync(core_);
        }, Qt::QueuedConnection);
    }

signals:
    void Notify();

private:
    void Wall(const EventCoreWeakPtr &syncCore) {
        if (syncCore.lock() != core_.lock()) {
            return ;
        }

        emit Notify();
    }

private:
    EventCoreWeakPtr core_;
    HSlot notifyUpdateSlot_;
};


}


#endif // __HCATEVENTTOQTEVENT_H__

#endif
