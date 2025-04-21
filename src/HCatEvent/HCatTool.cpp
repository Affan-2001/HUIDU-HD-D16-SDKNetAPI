

#include <HCatTool.h>

#include <clocale>
#include <locale>
#include <iostream>
#include <vector>

#ifndef RK_ANDROID
#include <codecvt>
#endif

#ifdef H_WIN
#include <windows.h>
#endif


using namespace cat;


std::string HCatTool::UnicodeToUTF8(const std::wstring &wstr)
{
#ifdef RK_ANDROID
    std::string ret(wstr.begin(), wstr.end());
#else
    std::string ret;
    try {
        std::wstring_convert< std::codecvt_utf8<wchar_t> > wcv;
        ret = wcv.to_bytes(wstr);
    } catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
    }
#endif
    return ret;
}

std::wstring HCatTool::UTF8ToUnicode(const std::string &str)
{
#ifdef RK_ANDROID
    std::wstring ret(str.begin(), str.end());
#else
    std::wstring ret;
    try {
        std::wstring_convert< std::codecvt_utf8<wchar_t> > wcv;
        ret = wcv.from_bytes(str);
    } catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
    }
#endif
    return ret;
}

std::string HCatTool::UnicodeToANSI(const std::wstring &wstr)
{
#ifdef RK_ANDROID
    std::string ret(wstr.begin(), wstr.end());
#else
    std::string ret;
    std::mbstate_t state = {};
    const wchar_t *src = wstr.data();
    size_t len = std::wcsrtombs(nullptr, &src, 0, &state);
    if (static_cast<size_t>(-1) != len) {
        std::vector<char> buff(len + 1);
        len = std::wcsrtombs(&buff[0], &src, len, &state);
        ret.assign(buff.begin(), buff.end() - 1);
    }
#endif
    return ret;
}

std::wstring HCatTool::ANSIToUnicode(const std::string &str)
{
#ifdef RK_ANDROID
    std::wstring ret(str.begin(), str.end());
#else
    std::wstring ret;
    std::mbstate_t state = {};
    const char *src = str.data();
    size_t len = std::mbsrtowcs(nullptr, &src, 0, &state);
    if (static_cast<size_t>(-1) != len) {
        std::vector<wchar_t> buff(len + 1);
        std::mbsrtowcs(&buff[0], &src, len, &state);
        ret.assign(buff.begin(), buff.end() - 1);
    }
#endif
    return ret;
}

std::string HCatTool::UTF8ToANSI(const std::string &str)
{
#ifdef H_WIN
    int len = MultiByteToWideChar(CP_UTF8, 0, str.data(), -1, NULL, 0);
    std::vector<wchar_t> wstr(len + 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), -1, wstr.data(), len);

    len = WideCharToMultiByte(CP_ACP, 0, wstr.data(), -1, NULL, 0, NULL, NULL);

    std::vector<char> dest(len + 1, 0);
    WideCharToMultiByte(CP_ACP, 0, &wstr[0], -1, dest.data(), len, NULL, NULL);

    return std::string(dest.data(), len);
#elif RK_ANDROID
    return str;
#else
    return UnicodeToANSI(UTF8ToUnicode(str));
#endif

}

std::string HCatTool::ANSIToUTF8(const std::string &str)
{
#ifdef H_WIN
    int len = MultiByteToWideChar(CP_ACP, 0, str.data(), -1, nullptr, 0);
    std::vector<wchar_t> wstr(len + 1, 0);
    MultiByteToWideChar(CP_ACP, 0, str.data(), -1, wstr.data(), len);

    len = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), -1, nullptr, 0, nullptr, nullptr);

    std::vector<char> dest(len + 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), -1, dest.data(), len, nullptr, nullptr);

    return std::string(dest.data(), len);
#else
    return UnicodeToUTF8(ANSIToUnicode(str));
#endif
}


std::vector<std::string> HCatTool::Split(const std::string &str, const std::string &delim)
{
    std::vector<std::string> result;
    std::size_t start = 0;
    std::size_t end = str.find(delim, start);
    while (end != std::string::npos) {
        if (start != end) {
            result.emplace_back(std::move(str.substr(start, end - start)));
        } else {
            result.emplace_back(std::string());
        }

        start = end + delim.size();
        end = str.find(delim, start);
    }

    result.emplace_back(std::move(str.substr(start)));
    return result;
}

std::string HCatTool::Simplified(const std::string &str)
{
    if (str.empty()) {
        return str;
    }

    std::string data(str);
    data.erase(0, data.find_first_not_of(" \t\r\n"));
    data.erase(data.find_last_not_of(" \t\r\n") + 1);
    return data;
}
