

#ifndef __HCATTHREAD_H__
#define __HCATTHREAD_H__


#include <functional>
#include <list>
#include <mutex>
#include <memory>
#include <thread>
#include <condition_variable>


namespace cat
{


/**
 * @brief The HCatThread class 线程, 任务直接Run丢入即可
 */
class HCatThread
{
public:
    HCatThread(const HCatThread &) = delete;
    HCatThread &operator =(const HCatThread &) = delete;

    HCatThread() : threadRun_(true), wordRun_(false) {
        thread_.reset(new std::thread(&HCatThread::Loop, this));
    }

    ~HCatThread() { this->Quit(); }

    bool IsRun() const { return wordRun_; }
    size_t Size() const { return taskList_.size(); }

    template <typename _Obj, typename _Func = typename _Obj::_Func, typename ..._Args>
    void Run(_Obj *obj, _Func func, _Args &&...args) { Push(std::bind(func, obj, args...)); }
    template <typename _Obj, typename _Func = typename _Obj::_Func>
    void Run(_Obj *obj, _Func func) { Push(std::bind(func, obj)); }

    template <typename... _Args>
    void Run(void(*func)(_Args...), _Args &&...args) { Push(std::bind(func, args...)); }

    void Run(std::function<void()> &&func) { Push(std::forward<std::function<void()>>(func)); }

    void Start() {
        if (thread_ || threadRun_) {
            return ;
        }

        threadRun_ = true;
        thread_.reset(new std::thread(&HCatThread::Loop, this));
    }

    void Quit() {
        {
            std::unique_lock<std::mutex> lock(lock_);
            threadRun_ = false;
            wait_.notify_all();
        }

        if (!thread_ || !thread_->joinable()) {
            return ;
        }

        thread_->join();
        thread_.reset();
    }
private:
    void Push(std::function<void()> &&func) {
        std::unique_lock<std::mutex> lock(lock_);
        taskList_.emplace_back(std::forward<std::function<void()>>(func));
        wait_.notify_all();
    }

    void Loop() {
        for (;;) {
            {
                std::unique_lock<std::mutex> lock(lock_);
                wait_.wait(lock, [this](){ return threadRun_ == false || taskList_.empty() == false; });
            }

            if (threadRun_ == false) {
                return ;
            }

            std::list<std::function<void()>> taskList;
            {
                std::unique_lock<std::mutex> lock(lock_);
                taskList.swap(taskList_);
            }

            for (const auto &func : taskList) {
                if (threadRun_ == false) {
                    return ;
                }

                wordRun_ = true;
                func();
                wordRun_= false;
            }
        }
    }

private:
    std::list<std::function<void()>> taskList_;
    std::unique_ptr<std::thread> thread_;
    std::mutex lock_;
    std::condition_variable wait_;
    bool threadRun_;
    bool wordRun_;
};


class HCatThreadPool
{
public:
    explicit HCatThreadPool(int maxThreadNum) : maxThread_(maxThreadNum) { if (maxThread_ < 1) { maxThread_ = 1;} }
    ~HCatThreadPool() { threadPool_.clear(); }
    HCatThreadPool(const HCatThreadPool &) = delete;
    HCatThreadPool &operator =(const HCatThreadPool &) = delete;

    template <typename _Obj, typename _Func, typename ..._Args>
    void Run(_Obj *obj, _Func func, _Args &&...args) { Push(std::bind(func, obj, args...)); }
    template <typename _Obj, typename _Func>
    void Run(_Obj *obj, _Func func) { Push(std::bind(func, obj)); }

    template <typename... _Args>
    void Run(void(*func)(_Args...), _Args &&...args) { Push(std::bind(func, args...)); }

    void Run(std::function<void()> &&func) { Push(std::forward<std::function<void()>>(func)); }

private:
    void Push(std::function<void()> &&func) {
        std::unique_lock<std::mutex> lock(lock_);
        taskList_.emplace_back(std::forward<std::function<void()>>(func));
        this->ParseTask();
    }

    void ParseTask() {
        if (threadPool_.empty()) {
            threadPool_.emplace_back(std::unique_ptr<HCatThread>(new HCatThread));
        }

        bool find = false;
        for (auto &i : threadPool_) {
            if (i->IsRun()) {
                continue;
            }

            find = true;
            i->Run(this, &HCatThreadPool::WordThread);
        }

        if (find || threadPool_.size() >= maxThread_) {
            return ;
        }

        threadPool_.emplace_back(std::unique_ptr<HCatThread>(new HCatThread));
        threadPool_.back()->Run(this, &HCatThreadPool::WordThread);
    }

    void WordThread() {
        std::function<void()> task;
        for (;;) {
            {
                std::unique_lock<std::mutex> lock(lock_);
                if (taskList_.empty()) {
                    return ;
                }
                task = std::move(taskList_.front());
                taskList_.pop_front();
            }

            task();
        }
    }

private:
    std::list<std::unique_ptr<HCatThread>> threadPool_;
    std::list<std::function<void()>> taskList_;
    std::mutex lock_;
    uint32_t maxThread_;
};



}


#endif // __HCATTHREAD_H__
