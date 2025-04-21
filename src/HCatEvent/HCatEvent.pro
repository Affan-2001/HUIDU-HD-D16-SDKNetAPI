CONFIG -= qt

TEMPLATE = lib
CONFIG += static
DEFINES += HCATEVENT_LIBRARY

OUT_NAME = HCatEvent
contains(CONFIG, static) {
OUT_NAME = HCatEvent_static
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
    HCatEventCore.cpp \
    HCatGcEvent.cpp \
    HCatIOEvent.cpp \
    HCatIOReadWriteEvent.cpp \
    HCatMainEvent.cpp \
    HCatNet/HCatSerial.cpp \
    HCatNet/HCatTcpService.cpp \
    HCatNet/HCatTcpSocket.cpp \
    HCatNet/HCatUdpSocket.cpp \
    HCatThreadTaskEvent.cpp \
    HCatTimeEvent.cpp \
    HCatTool.cpp \
    ICatEventApi.cpp

HEADERS += \
    HCatAny.h \
    HCatBuffer.h \
    HCatEventCore.h \
    HCatEventInfo.h \
    HCatEventStruct.h \
    HCatForward.h \
    HCatGcEvent.h \
    HCatIOEvent.h \
    HCatIOReadWriteEvent.h \
    HCatMainEvent.h \
    HCatNet/HCatSerial.h \
    HCatNet/HCatTcpService.h \
    HCatNet/HCatTcpSocket.h \
    HCatNet/HCatUdpSocket.h \
    HCatSignal.h \
    HCatSignalSet.h \
    HCatThread.h \
    HCatThreadTaskEvent.h \
    HCatTimeEvent.h \
    HCatTimer.h \
    HCatTool.h \
    ICatEventApi.h \
    ICatEventBase.h \
    ICatEventData.h

