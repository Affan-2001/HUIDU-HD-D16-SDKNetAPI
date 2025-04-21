CONFIG -= qt

TEMPLATE = lib
#CONFIG += static
DEFINES += HCATNET_LIBRARY

# 这个是会话Debug信号日志输出
DEFINES += DEBUG_LOG_SIGNAL

OUT_NAME = HCatNet
contains(CONFIG, static) {
OUT_NAME = HCatNet_static
}

win32:CONFIG(debug, debug|release): CONFIG(debug, debug|release): TARGET = $$OUT_NAME"d"
else:win32:CONFIG(release, debug|release): TARGET = $$OUT_NAME
else:unix:!macx: TARGET = $$OUT_NAME

contains(QT_ARCH, i386) {
    DESTDIR = ../bin
} else {
    DESTDIR = ../bin64
}

SOURCES += \
    HCApiImpl.cpp \
    HCatSyncCoreEvent.cpp \
    ICApi.cpp \
    IEventApi.cpp \
    ISession.cpp \
    Net/HSerialNet.cpp \
    Net/HTcpNet.cpp \
    Net/HTcpServiceNet.cpp \
    Net/HUdpNet.cpp \
    Protocol/HHexProtocol.cpp \
    Protocol/HHttpProtocol.cpp \
    Protocol/HLcdSdkProtocol.cpp \
    Protocol/HOldSDKProtocol.cpp \
    Protocol/HRawStringProtocol.cpp \
    Protocol/HSDKProtocol.cpp \
    Protocol/HSerialSDKProtocol.cpp \
    Protocol/HStruct.cpp \
    Protocol/HUdpFindDeviceProtocol.cpp \
    Tool/HTool.cpp \
    Tool/md5.cpp \
    Tool/tinyxml2.cpp

HEADERS += \
    CatString.h \
    HCApiImpl.h \
    HCatEventToQtEvent.h \
    HCatNetInfo.h \
    HCatSyncCoreEvent.h \
    HQtThread.h \
    ICApi.h \
    ICApiStatic.h \
    IEventApi.h \
    ISession.h \
    Net/HSerialNet.h \
    Net/HTcpNet.h \
    Net/HTcpServiceNet.h \
    Net/HUdpNet.h \
    Net/INetBase.h \
    Protocol/HHexProtocol.h \
    Protocol/HHttpProtocol.h \
    Protocol/HLcdSdkProtocol.h \
    Protocol/HOldSDKProtocol.h \
    Protocol/HRawStringProtocol.h \
    Protocol/HSDKProtocol.h \
    Protocol/HSerialSDKProtocol.h \
    Protocol/HStruct.h \
    Protocol/HUdpFindDeviceProtocol.h \
    Protocol/IProtocolBase.h \
    Tool/HTool.h \
    Tool/md5.h \
    Tool/tinyxml2.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target


INCLUDEPATH += $$PWD/../HCatEvent
DEPENDPATH += $$PWD/../HCatEvent

contains(CONFIG, static) {
    DEFINES += HCATEVENT_LIBRARY
    include($$PWD/../HCatEvent/HCatEvent.pri)
} else {
    HCATEVENT_NAME = HCatEvent
    exists($$DESTDIR/HCatEvent_static.lib) {
        HCATEVENT_NAME = HCatEvent_static
    }
    exists($$DESTDIR/HCatEvent_staticd.lib) {
        HCATEVENT_NAME = HCatEvent_static
    }

    win32:CONFIG(release, debug|release): LIBS += -L$$DESTDIR/ -l$$HCATEVENT_NAME
    else:win32:CONFIG(debug, debug|release): LIBS += -L$$DESTDIR/ -l$$HCATEVENT_NAME"d"
    else:unix:!macx: LIBS += -L$$DESTDIR/ -l$$HCATEVENT_NAME

    win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$DESTDIR/$$HCATEVENT_NAME".a"
    else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$DESTDIR/$$HCATEVENT_NAME".a"
    else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$DESTDIR/$$HCATEVENT_NAME".lib"
    else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$DESTDIR/$$HCATEVENT_NAME".lib"
    else:unix:!macx: PRE_TARGETDEPS += $$DESTDIR/$$HCATEVENT_NAME".a"
}

unix:!RK_ANDROID {
LIBS += -luuid
}
