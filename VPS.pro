QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QT_CONFIG -= no-pkg-config
CONFIG += c++11 link_pkgconfig
CONFIG -= app_bunddle
#PKGCONFIG += opencv4

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += /usr/local/include/
LIBS += -L/usr/local/lib

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Mirroring/MIR_Common.cpp \
    Mirroring/mir_client.cpp \
    Mirroring/mir_mempool.cpp \
    Mirroring/mir_queue.cpp \
    Mirroring/mir_queue2.cpp \
    Mirroring/mir_queuehandler.cpp \
    Mirroring/mirroring.cpp \
    Network/asyncmediaserversocket.cpp \
    Network/clientobject.cpp \
    Network/netmanager.cpp \
    ThreadHelper.cpp \
    VpsJpeg/VpsJpegLib.cpp \
    VpsJpeg/vpsjpeg.cpp \
    captureahybrid.cpp \
    main.cpp \
    vps.cpp

HEADERS += \
    Mirroring/MIR_Common.h \
    Mirroring/MirroringCallback.h \
    Mirroring/mir_client.h \
    Mirroring/mir_mempool.h \
    Mirroring/mir_queue.h \
    Mirroring/mir_queue2.h \
    Mirroring/mir_queuehandler.h \
    Mirroring/mirroring.h \
    Network/asyncmediaserversocket.h \
    Network/clientobject.h \
    Network/netmanager.h \
    ThreadHelper.h \
    VPSCommon.h \
    VpsJpeg/VpsJpegLib.h \
    VpsJpeg/vpsjpeg.h \
    captureahybrid.h \
    vps.h

FORMS += \
    vps.ui

TRANSLATIONS += \
    VPS_ko_KR.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
