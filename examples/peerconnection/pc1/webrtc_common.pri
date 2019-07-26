CONFIG += no_keywords

INCLUDEPATH += \
        $$PWD\..\..\..\webrtc\src\third_party\libyuv\include \
        $$PWD\..\..\..\webrtc\src \
        $$PWD\..\..\..\webrtc\src\third_party\abseil-cpp

#$$PWD\..\..\..\webrtc\src\third_party\jsoncpp\source\include \

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

    contains(QT_ARCH, x86_64) {
        message("x64")
    } else {
        message("x86")

        CONFIG(debug, debug|release) {
            DESTDIR = $$PWD/../../../exampleout/win32/debug/pc1
            LIBS += \
                -L$$PWD\..\..\..\out\debug\obj -lwebrtc \
                -L$$PWD\..\..\..\out\debug\obj\api -lcreate_peerconnection_factory \
                -L$$PWD\..\..\..\out\debug\obj\rtc_base -lrtc_base
        } else {
            DESTDIR = $$PWD/../../../exampleout/win32/release/pc1
            LIBS += \
                -L$$PWD\..\..\..\out\release\obj -lwebrtc \
                -L$$PWD\..\..\..\out\release\obj\api -lcreate_peerconnection_factory \
                -L$$PWD\..\..\..\out\release\obj\rtc_base -lrtc_base
        }        
    }
}
