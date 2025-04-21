

#ifndef __HCATEVENT_CATTOOL_H__
#define __HCATEVENT_CATTOOL_H__


#include <HCatEventInfo.h>

#include <string>
#include <vector>
#include <sstream>


namespace cat
{



class HCATEVENT_EXPORT HCatTool
{
public:
    static std::string UnicodeToUTF8(const std::wstring &wstr);
    static std::wstring UTF8ToUnicode(const std::string &str);

    static std::string UnicodeToANSI(const std::wstring &wstr);
    static std::wstring ANSIToUnicode(const std::string &str);

    static std::string UTF8ToANSI(const std::string &str);
    static std::string ANSIToUTF8(const std::string &str);

    ///< 切割字符串
    static std::vector<std::string> Split(const std::string& str, const std::string &delim = " ");

    template <typename _Ret, typename _Value>
    static _Ret ToType(const _Value &value) {
        std::stringstream ss;
        ss << value;
        _Ret result;
        ss >> result;
        return result;
    }

    // 删除前后空格
    static std::string Simplified(const std::string &str);
};


template <typename _T>
class RallPoint
{
public:
    using value_p = _T *;
    using const_value_r = const _T &;

    RallPoint(value_p value, const_value_r init, const_value_r destory) : bind_(value), destory_(destory) {
        if (bind_) {
            *bind_ = init;
        }
    }
    RallPoint(value_p value, const_value_r destory) : bind_(value), destory_(destory) {}
    ~RallPoint() {
        if (bind_) {
            *bind_ = destory_;
        }
    }

    _T *Take() {
        _T *tmp = bind_;
        bind_ = nullptr;
        return tmp;
    }

    void Reset(_T *value, _T destory) {
        if (bind_) {
            *bind_ = destory_;
        }
        bind_ = value;
        destory_ = destory;
    }

private:
    _T *bind_;
    _T destory_;
};

}




#endif // __HCATEVENT_CATTOOL_H__
