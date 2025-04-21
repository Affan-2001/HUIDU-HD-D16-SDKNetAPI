

#ifndef __BASIC_CORE_ANY_H__
#define __BASIC_CORE_ANY_H__


#include <memory>
#include <typeindex>
#include <type_traits>
#include <typeinfo>


namespace cat
{


class HCatAny
{
private:
    struct DataBase;
    using dataPtr = std::unique_ptr<DataBase>;

    struct DataBase
    {
        virtual ~DataBase() {}
        virtual dataPtr Clone() const = 0;
        virtual const std::type_info &Type() const = 0;
    };

    template<typename _T>
    struct Data : public DataBase
    {
        template<typename...Args>
        Data(Args&&...args) : data(std::forward<Args>(args)...){ }

        virtual dataPtr Clone() const override { return dataPtr(new Data(data)); }
        virtual const std::type_info &Type() const override { return typeid(_T); }

        _T data;
    };

    dataPtr Clone() const {
        if (data_) {
            return data_->Clone();
        }
        return nullptr;
    }

public:
    HCatAny() {}
    HCatAny(const HCatAny &other) : data_(other.Clone()) {}
    HCatAny(HCatAny &&other) : data_(std::move(other.data_)){}

    template <typename _T, typename = typename std::enable_if<!std::is_same<typename std::decay<_T>::type, HCatAny>::value, _T>::type>
    HCatAny(_T &&t) : data_(new Data<typename std::decay<_T>::type>(std::forward<_T>(t))) {}


    HCatAny &Swap(HCatAny &other) { data_.swap(other.data_); return *this; }

    bool IsEmpty() const { return !data_; }
    bool IsNUll() const { return IsEmpty(); }
    bool HasValue() const { return !IsEmpty(); }
    bool IsVaild() const { return HasValue(); }

    const std::type_info &Type() const { return data_ ?  data_->Type() : typeid(void); }

    template <typename _T>
    bool IsType() const { return Type() == typeid(_T); }


    void Clear() { dataPtr().swap(data_); }

    ///< 如使用之前未使用IsType检查是否是同一类型, 那么请注意std::bad_cast异常
    template <typename _T>
    _T& Value() const {
        if (IsType<_T>() == false) {
            throw std::bad_cast();
        }

        auto p = dynamic_cast<Data<_T>*>(this->data_.get());
        return p->data;
    }

    template <typename _T>
    _T& Cast() const {
        auto p = reinterpret_cast<Data<_T>*>(this->data_.get());
        return p->data;
    }

    HCatAny &operator=(const HCatAny &other) {
        if (this->data_ == other.data_) {
            return *this;
        }

        this->data_ = other.Clone();
        return *this;
    }

    HCatAny &operator=(HCatAny &&other) {
        this->data_ = std::move(other.data_);
        return *this;
    }


private:
    dataPtr data_;
};


}


#endif // __BASIC_CORE_ANY_H__
