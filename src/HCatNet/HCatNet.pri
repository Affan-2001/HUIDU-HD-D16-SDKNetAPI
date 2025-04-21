

# 这个是会话Debug信号日志输出
DEFINES += DEBUG_LOG_SIGNAL


SOURCES += \
    $$PWD/../HCatNet/HCApiImpl.cpp \
    $$PWD/../HCatNet/HCatSyncCoreEvent.cpp \
    $$PWD/../HCatNet/ICApi.cpp \
    $$PWD/../HCatNet/IEventApi.cpp \
    $$PWD/../HCatNet/ISession.cpp \
    $$PWD/../HCatNet/Net/HSerialNet.cpp \
    $$PWD/../HCatNet/Net/HTcpNet.cpp \
    $$PWD/../HCatNet/Net/HTcpServiceNet.cpp \
    $$PWD/../HCatNet/Net/HUdpNet.cpp \
    $$PWD/../HCatNet/Protocol/HHexProtocol.cpp \
    $$PWD/../HCatNet/Protocol/HHttpProtocol.cpp \
    $$PWD/../HCatNet/Protocol/HLcdSdkProtocol.cpp \
    $$PWD/../HCatNet/Protocol/HOldSDKProtocol.cpp \
    $$PWD/../HCatNet/Protocol/HRawStringProtocol.cpp \
    $$PWD/../HCatNet/Protocol/HSDKProtocol.cpp \
    $$PWD/../HCatNet/Protocol/HSerialSDKProtocol.cpp \
    $$PWD/../HCatNet/Protocol/HStruct.cpp \
    $$PWD/../HCatNet/Protocol/HUdpFindDeviceProtocol.cpp \
    $$PWD/../HCatNet/Tool/HTool.cpp \
    $$PWD/../HCatNet/Tool/md5.cpp \
    $$PWD/../HCatNet/Tool/tinyxml2.cpp

HEADERS += \
    $$PWD/../HCatNet/CatString.h \
    $$PWD/../HCatNet/HCApiImpl.h \
    $$PWD/../HCatNet/HCatEventToQtEvent.h \
    $$PWD/../HCatNet/HCatNetInfo.h \
    $$PWD/../HCatNet/HCatSyncCoreEvent.h \
    $$PWD/../HCatNet/HQtThread.h \
    $$PWD/../HCatNet/ICApi.h \
    $$PWD/../HCatNet/IEventApi.h \
    $$PWD/../HCatNet/ISession.h \
    $$PWD/../HCatNet/Net/HSerialNet.h \
    $$PWD/../HCatNet/Net/HTcpNet.h \
    $$PWD/../HCatNet/Net/HTcpServiceNet.h \
    $$PWD/../HCatNet/Net/HUdpNet.h \
    $$PWD/../HCatNet/Net/INetBase.h \
    $$PWD/../HCatNet/Protocol/HHexProtocol.h \
    $$PWD/../HCatNet/Protocol/HHttpProtocol.h \
    $$PWD/../HCatNet/Protocol/HLcdSdkProtocol.h \
    $$PWD/../HCatNet/Protocol/HOldSDKProtocol.h \
    $$PWD/../HCatNet/Protocol/HRawStringProtocol.h \
    $$PWD/../HCatNet/Protocol/HSDKProtocol.h \
    $$PWD/../HCatNet/Protocol/HSerialSDKProtocol.h \
    $$PWD/../HCatNet/Protocol/HStruct.h \
    $$PWD/../HCatNet/Protocol/HUdpFindDeviceProtocol.h \
    $$PWD/../HCatNet/Protocol/IProtocolBase.h \
    $$PWD/../HCatNet/Tool/HTool.h \
    $$PWD/../HCatNet/Tool/md5.h \
    $$PWD/../HCatNet/Tool/tinyxml2.h



INCLUDEPATH += $$PWD/../HCatEvent
DEPENDPATH += $$PWD/../HCatEvent
INCLUDEPATH += $$PWD/../HCatNet
DEPENDPATH += $$PWD/../HCatNet

win32:CONFIG(release, debug|release): LIBS += -L$$DESTDIR/ -lHCatEvent
else:win32:CONFIG(debug, debug|release): LIBS += -L$$DESTDIR/ -lHCatEventd
else:unix:!macx: LIBS += -L$$DESTDIR/ -lHCatEvent

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$DESTDIR/HCatEvent.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$DESTDIR/HCatEventd.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$DESTDIR/HCatEvent.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$DESTDIR/HCatEventd.lib
else:unix:!macx: PRE_TARGETDEPS += $$DESTDIR/libHCatEvent.a


unix:!RK_ANDROID {
LIBS += -luuid
}
