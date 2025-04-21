

#ifndef __HCATFORWARD_H__
#define __HCATFORWARD_H__


#include <ICatEventApi.h>
#include <HCatEventStruct.h>


namespace cat
{


/**
 * @brief The HCatForward class 这是一个封装了将任务转发到其他核心的转发器
 * 但它无法保证生存期调用的安全. 必须优先在析构时调用取消任务.
 */
class HCatForward : public HCatEventObject
{
public:
    template <typename _Obj, typename _Func = typename _Obj::_Func, typename... _Args>
    void Forward(const EventCoreWeakPtr &core, _Obj *obj, _Func func, _Args &&...args) const {
        ICatEventApi::Forward(core, this, std::bind(func, obj, std::forward<_Args>(args)...));
    }

    void Forward(const EventCoreWeakPtr &core, std::function<void()> &&func) {
        ICatEventApi::Forward(core, this, std::move(func));
    }

    void Quit() {
        this->RebirthID();
    }
};


}


#endif // __HCATFORWARD_H__
