QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

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
    main.cpp \
    main_wnd.cpp \
    defaults.cc \
    peer_connection_client.cc \
    conductor.cc \
    videorenderer.cpp

HEADERS += \
    main_wnd.h \
    flag_defs.h \
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

include($$PWD/webrtc_common.pri)
include($$PWD/test/test.pri)

INCLUDEPATH += $$PWD/test

win32 {
    LIBS += Ole32.lib OleAut32.lib User32.lib Ws2_32.lib
}

macos {
    LIBS += -framework CoreMedia \
            -framework AVFoundation \
            -framework CoreVideo

    QMAKE_INFO_PLIST = $$PWD/Info.plist

    CONFIG(debug, debug|release) {
        LIBS += \
            -L$$PWD/../../../../out/debug/obj/sdk -lvideocapture_objc \
            -L$$PWD/../../../../out/debug/obj/sdk -lvideoframebuffer_objc \
            -L$$PWD/../../../../out/debug/obj/sdk -lbase_objc \
            -L$$PWD/../../../../out/debug/obj/sdk -lnative_video
    } else {
        LIBS += \
            -L$$PWD/../../../../out/release/obj/sdk -lvideocapture_objc \
            -L$$PWD/../../../../out/release/obj/sdk -lvideoframebuffer_objc \
            -L$$PWD/../../../../out/release/obj/sdk -lbase_objc \
            -L$$PWD/../../../../out/release/obj/sdk -lnative_video
    }
}
