

#ifndef __HCATSIGNALSET_H__
#define __HCATSIGNALSET_H__


#include <HCatSignal.h>

#include <list>


namespace cat
{


/**
 * @brief The HCatSignalSet class 给不取消信号的功能提供一个槽集合存放
 */
class HCatSignalSet
{
public:
    HCatSignalSet() {}

    HCatSignalSet &Append(const HSlot &data) { slots_.push_back(data); return *this; }
    HCatSignalSet &Append(HSlot &&data) { slots_.emplace_back(data); return *this; }

    HCatSignalSet &operator= (const HSlot &data) { this->Append(data); return *this; }
    HCatSignalSet &operator= (HSlot &&data) { this->Append(data); return *this; }

    HCatSignalSet &operator+=(const HSlot &data) { return this->Append(data); }

    void Clear() { slots_.clear(); }

private:
    std::list<HSlot> slots_;
};



}


#endif // HCATSIGNALSET_H
