QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    bufferutil.cpp \
    controlmsg.cpp \
    main.cpp \
    main_wnd.cpp \
    defaults.cc \
    peer_connection_client.cc \
    conductor.cc \
    videorenderer.cpp

HEADERS += \
    bufferutil.h \
    controlmsg.h \
    main_wnd.h \
    defaults.h \
    peer_connection_client.h \
    conductor.h \
    videorenderer.h

FORMS += \
    mainwnd.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

include($$PWD/../../webrtc_common/webrtc_common.pri)
include($$PWD/test/test.pri)
include($$PWD/render/render.pri)

INCLUDEPATH += \
            $$PWD/test \
            $$PWD/render

win32 {
    SOURCES += \
        inputinject.cpp

    HEADERS += \
        inputinject.h
}

macos {
    QMAKE_INFO_PLIST = $$PWD/Info.plist
}
