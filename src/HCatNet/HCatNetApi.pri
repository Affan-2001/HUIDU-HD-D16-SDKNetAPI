

# 提供事件接口所需的头文件和一些头文件库

include($$PWD/../HCatEvent/HCatApi.pri)

HEADERS += \
    $$PWD/ISession.h \
    $$PWD/HCatSyncCoreEvent.h \
    $$PWD/HCatEventToQtEvent.h \
    $$PWD/HCatNetInfo.h \
    $$PWD/IEventApi.h

INCLUDEPATH += $$PWD/../HCatEvent/
INCLUDEPATH += $$PWD/../HCatNet/
