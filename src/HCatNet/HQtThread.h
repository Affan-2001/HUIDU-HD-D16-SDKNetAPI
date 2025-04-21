

#ifndef HCATNET_LIBRARY


#ifndef __BASIC_CORE_QTTHREAD_H__
#define __BASIC_CORE_QTTHREAD_H__


#include <QTimer>
#include <QThread>
#include <functional>
#include <QList>
#include <QMutex>
#include <QSharedPointer>


namespace cat
{


/**
 * @brief The HQtThreadTask class 用于Qt5的 moveToThread线程
 */
class HQtThreadTask : public QObject
{
    ///< 这里不使用Q_OBJECT, 是为了跨文件调用, 包含头文件就可以使用了
    ///< Q_OBJECT会生成moc文件, 但这个不需要, 这样会导致连接问题
    ///< 触发线程使用匿名函数触发, 不需要moc来实现槽
public:
    HQtThreadTask() : run_(false) {}
    HQtThreadTask(const HQtThreadTask &) = delete;
    HQtThreadTask &operator =(const HQtThreadTask &) = delete;

    template <typename _Obj, typename _Func, typename ..._Args>
    void Run(_Obj *obj, _Func func, _Args &&...args) {
        auto f = std::bind(func, obj, args...);
        QTimer::singleShot(0, this, [f, this]() { run_ = true; f(); run_ = false; });
    }

    template <typename _Obj, typename _Func>
    void Run(_Obj *obj, _Func func) {
        auto f = std::bind(func, obj);
        QTimer::singleShot(0, this, [f, this]() { run_ = true; f(); run_ = false; });
    }

    void Run(const std::function<void()> &func) {
        QTimer::singleShot(0, this, [func, this]() { run_ = true; func(); run_ = false; });
    }

    bool IsRun() const { return run_; }

private:
    bool run_;
};


/**
 * @brief The HQtThread class Qt5线程任务, 只需构建这个对象就可以使用Qt线程了
 */
class HQtThread
{
public:
    HQtThread(): thread_(new QThread), task_(new HQtThreadTask) {
        task_->moveToThread(thread_.data());
        thread_->start();
    }
    HQtThread(const HQtThread &) = delete;
    HQtThread &operator =(const HQtThread &) = delete;

    ~HQtThread() {
        this->Quit();
        task_->deleteLater();
    }

    ///< 需要在线程执行的任务将在此处运行
    template <typename _Obj, typename _Func, typename ..._Args>
    void Run(_Obj *obj, _Func func, _Args &&...args) {
        task_->Run(obj, func, std::forward<_Args>(args)...);
    }
    template <typename _Obj, typename _Func>
    void Run(_Obj *obj, _Func func) {
        task_->Run(obj, func);
    }

    void Run(const std::function<void()> &func) {
        task_->Run(func);
    }

    ///< 当前线程是否有任务正在进行
    bool IsRun() const { return task_->IsRun(); }

    ///< 强制终止线程
    void Terminate() { thread_->terminate(); }

    ///< 开始线程, 构造函数的时候已运行
    void Start() {
        if (thread_ && thread_->isRunning()) {
            return ;
        }

        if (!thread_) {
            thread_.reset(new QThread());
        }

        task_->moveToThread(thread_.data());
        thread_->start();
    }

    ///< 退出线程
    void Quit() {
        if (thread_) {
            thread_->quit();
            thread_->wait();
            thread_.reset();
        }
    }
private:
    QScopedPointer<QThread> thread_;
    QScopedPointer<HQtThreadTask> task_;
};



/**
 * @brief The HQtThreadPool class 封装HQtThread的线程池
 */
class HQtThreadPool
{
public:
    HQtThreadPool(uint32_t threadNum = 4) : threadNum_(threadNum) { }
    HQtThreadPool(const HQtThreadPool &) = delete;
    HQtThreadPool &operator =(const HQtThreadPool &) = delete;
    ~HQtThreadPool() {
        taskPool_.clear();
        threadPool_.clear();
    }

    template <typename _Obj, typename _Func, typename ..._Args>
    void Run(_Obj *obj, _Func func, _Args &&...args) {
        auto f = std::bind(func, obj, args...);
        Push(f);
    }

    template <typename _Obj, typename _Func>
    void Run(_Obj *obj, _Func func) {
        auto f = std::bind(func, obj);
        Push(f);
    }

    void Run(std::function<void()> func) {
        Push(func);
    }

    ///< 强制终止线程池
    void Terminate() const { for (const auto &i : threadPool_) { i->Terminate(); } }

private:
    void Push(const std::function<void()> &func) {
        QMutexLocker lock(&lock_);
        taskPool_.append(func);
        StartTask();
    }

    void StartTask() {
        if (threadPool_.isEmpty()) {
            threadPool_.append(QSharedPointer<HQtThread>(new HQtThread));
        }

        bool find = false;
        for (int i = 0; i < threadPool_.size(); ++i) {
            if (threadPool_.at(i)->IsRun()) {
                continue;
            }

            find = true;
            threadPool_.at(i)->Run(this, &HQtThreadPool::WordThread);
        }

        if (find || static_cast<uint32_t>(threadPool_.size()) >= threadNum_) {
            return ;
        }

        threadPool_.append(QSharedPointer<HQtThread>(new HQtThread));
        threadPool_.last()->Run(this, &HQtThreadPool::WordThread);
    }

    void WordThread() {
        std::function<void()> task;
        for (;;) {
            {
                QMutexLocker lock(&lock_);
                if (taskPool_.isEmpty()) {
                    return ;
                }
                task = taskPool_.first();
                taskPool_.removeFirst();
            }

            task();
        }
    }
private:
    QList<QSharedPointer<HQtThread>> threadPool_;
    QList<std::function<void()>> taskPool_;
    QMutex lock_;
    uint32_t threadNum_;
};


}



#endif // __BASIC_CORE_QTTHREAD_H__

#endif
