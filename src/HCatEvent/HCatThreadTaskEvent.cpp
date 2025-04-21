

#include <HCatThreadTaskEvent.h>


using namespace cat;


const uint32_t HCatThreadTaskEvent::MAX_THREAD_NUM = std::thread::hardware_concurrency() == 0 ? 4 : std::thread::hardware_concurrency()  * 2;


HCatThreadTaskEvent::HCatThreadTaskEvent(const EventCoreWeakPtr &core)
    : core_(core)
    , threadPool_(new HCatThreadPool(MAX_THREAD_NUM))
    , exit_(false)
{

}

HCatThreadTaskEvent::~HCatThreadTaskEvent()
{
    exit_ = true;
    threadPool_.reset();
}

void HCatThreadTaskEvent::Move(H_EventPtr &&event)
{
    if (event->id.IsInvalid() || core_.expired()) {
        return ;
    }

    switch (event->optEvent) {
    case kCall:
    case kRegister: {
        std::unique_lock<std::mutex> lock(lock_);
        taskList_.emplace_back(std::forward<H_EventPtr>(event));
        threadPool_->Run(this, &HCatThreadTaskEvent::ParseTask);
    } break;
    default:
        break;
    };

    return ;
}

void HCatThreadTaskEvent::ParseTask()
{
    H_EventPtr task;
    for (;;) {
        if (exit_ == true) {
            return ;
        }

        {
            std::unique_lock<std::mutex> lock(lock_);
            if (taskList_.empty()) {
                return ;
            }
            task = std::move(taskList_.front());
            taskList_.pop_front();
        }

        if (task->id.IsInvalid()) {
            continue;
        }

        if (task->callback) {
            task->callback();
        }
        task->optEvent = kCallEnd;

        auto core = core_.lock();
        if (!core) {
            return ;
        }
        core->Move(std::move(task));
    }
}
