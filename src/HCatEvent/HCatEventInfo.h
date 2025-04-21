#ifndef HCATEVENTINFO_H
#define HCATEVENTINFO_H


#if defined(_MSC_VER) || defined(WIN64) || defined(_WIN64) || defined(__WIN64__) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define H_WIN
#  define Q_DECL_EXPORT __declspec(dllexport)
#  define Q_DECL_IMPORT __declspec(dllimport)
#else
#define H_LINUX
#  define Q_DECL_EXPORT     __attribute__((visibility("default")))
#  define Q_DECL_IMPORT     __attribute__((visibility("default")))
#endif

// 使用静态库时, 不需要导出文件
#ifdef STATIC_IMPORT
#ifdef Q_DECL_IMPORT
#undef Q_DECL_IMPORT
#define Q_DECL_IMPORT
#endif
#endif

#if defined(HCATEVENT_LIBRARY)
#  define HCATEVENT_EXPORT Q_DECL_EXPORT
#else
#  define HCATEVENT_EXPORT Q_DECL_IMPORT
#endif


typedef unsigned long long      huint64;
typedef unsigned long           huint32l;
typedef unsigned int            huint32;
typedef unsigned short          huint16;
typedef unsigned char           huint8;

typedef long long               hint64;
typedef long                    hint32l;
typedef int                     hint32;
typedef short                   hint16;
typedef char                    hint8;
typedef float                   hfloat;
typedef double                  hdouble;


#ifndef UNUSED
# define UNUSED(x) (void)(x)
#endif


#endif // HCATEVENTINFO_H
