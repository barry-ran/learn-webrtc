SOURCES += \
        $$PWD/test_video_capturer.cc \
        $$PWD/platform_video_capturer.cc \

HEADERS += \
        $$PWD/test_video_capturer.h \
        $$PWD/platform_video_capturer.h

win32 {
    SOURCES += \
        $$PWD/vcm_capturer.cc

    HEADERS += \
        $$PWD/vcm_capturer.h
}

macos {
    SOURCES += \
        $$PWD/mac_capturer.mm

    HEADERS += \
        $$PWD/mac_capturer.h
}
