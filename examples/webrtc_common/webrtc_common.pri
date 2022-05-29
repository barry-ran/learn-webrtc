CONFIG += no_keywords

INCLUDEPATH += \
    $$PWD/../../webrtc/src/third_party/libyuv/include \
    $$PWD/../../webrtc/src \
    $$PWD/../../webrtc/src/third_party/abseil-cpp

BUILD_MODE = debug
CONFIG(debug, debug|release) {
    BUILD_MODE = debug
} else {
    BUILD_MODE = release
}

LIBS += \
    -L$$PWD/../../out/$$BUILD_MODE/obj -lwebrtc \
    -L$$PWD/../../out/$$BUILD_MODE/obj/api -lcreate_peerconnection_factory \
    -L$$PWD/../../out/$$BUILD_MODE/obj/rtc_base -lrtc_base

message($$LIBS)

# ***********************************************************
# Win平台下配置
# ***********************************************************
win32 {
    DEFINES += WEBRTC_WIN \
    WIN32_LEAN_AND_MEAN \
    USE_BUILTIN_SW_CODECS \
    HAVE_WEBRTC_VIDEO \
    INCL_EXTRA_HTON_FUNCTIONS \
    NOMINMAX

    LIBS += \
        -lwmcodecdspuuid \
        -ldmoguids \
        -lMsdmo \
        -lSecur32 \
        -lShell32 \
        -lGdi32 \
        -lWinmm \
        -lAdvapi32 \
        -lstrmiids \
        -lDXGI \
        -lD3D11 \
        -lWs2_32 \
        -lOle32 \
        -lOleAut32 \
        -lUser32

    PLANTFORM_MODE = win32
    contains(QT_ARCH, x86_64) {
        PLANTFORM_MODE = win64
    } else {
        PLANTFORM_MODE = win32
    }

    DESTDIR = $$PWD/../../exampleout/$$PLANTFORM_MODE/$$BUILD_MODE/$$TARGET
    DESTDIR = $$PWD/../../exampleout/$$PLANTFORM_MODE/$$BUILD_MODE/$$TARGET
}

# ***********************************************************
# Mac平台下配置
# ***********************************************************
macos {
    DEFINES += WEBRTC_MAC \
    WEBRTC_POSIX \
    USE_BUILTIN_SW_CODECS \
    HAVE_WEBRTC_VIDEO \
    INCL_EXTRA_HTON_FUNCTIONS

    # QMAKE_LFLAGS += -F /System/Library/Frameworks/CoreFoundation.framework/
    LIBS += \
        -framework CoreFoundation \
        -framework CoreGraphics \
        -framework IOSurface \
        -framework Foundation \
        -framework AppKit \
        -framework CoreMedia \
        -framework AVFoundation \
        -framework CoreVideo \
        -framework CoreAudio \
        -framework AudioToolbox

    LIBS += \
        -L$$PWD/../../out/$$BUILD_MODE/obj/sdk -lvideocapture_objc \
        -L$$PWD/../../out/$$BUILD_MODE/obj/sdk -lvideoframebuffer_objc \
        -L$$PWD/../../out/$$BUILD_MODE/obj/sdk -lbase_objc \
        -L$$PWD/../../out/$$BUILD_MODE/obj/sdk -lnative_video

    INCLUDEPATH += \
        $$PWD/../../webrtc/src/sdk/objc/base \
        $$PWD/../../webrtc/src/sdk/objc

    DESTDIR = $$PWD/../../exampleout/mac64/$$BUILD_MODE/$$TARGET
}
