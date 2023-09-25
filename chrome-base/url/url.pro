#-------------------------------------------------
#
# Project created by QtCreator 2016-01-29T18:33:57
#
#-------------------------------------------------

QT       -= core gui

TARGET = url_lib
TEMPLATE = lib

INCLUDEPATH += ../ ../third_party/icu/source/i18n \
    ../third_party/icu/source/common

DEFINES += COMPONENT_BUILD URL_IMPLEMENTATION USE_X11=1 \
    USE_SYMBOLIZE USE_OPENSSL=1 USE_GLIB=1 USE_NSS_CERTS=1 \
    __STDC_CONSTANT_MACROS __STDC_FORMAT_MACROS \
    U_USING_ICU_NAMESPACE=0 U_ENABLE_DYLOAD=0

QMAKE_CXXFLAGS += -fstack-protector --param=ssp-buffer-size=4 -Werror -pthread \
    -fno-strict-aliasing -Wall -Wextra -Wno-unused-parameter \
    -Wno-missing-field-initializers -fvisibility=hidden -pipe -fPIC \
    -fno-exceptions -fno-rtti -fno-threadsafe-statics \
    -fvisibility-inlines-hidden -std=gnu++11 \
    -m64 -march=x86-64

QMAKE_LFLAGS += -Wl,-z,now -Wl,-z,relro -Wl,--fatal-warnings -Wl,-z,defs -pthread \
    -Wl,-z,noexecstack -fPIC -fuse-ld=gold -m64 -Wl,--disable-new-dtags \
    -Wl,--detect-odr-violations -Wl,--icf=all

LIBS += -lrt -ldl

CONFIG(debug, debug|release) {
DEFINES += DYNAMIC_ANNOTATIONS_ENABLED=1 _DEBUG _GLIBCXX_DEBUG=1

QMAKE_CXXFLAGS += -O0 -g

LIBS += $$OUT_PWD/../lib/debug/libdynamic_annotations.a \
    -L$$PWD/../lib/debug -lbase \
    -L$$PWD/../lib/debug -licuuc -licui18n

DESTDIR = $$PWD/../lib/debug/
}

CONFIG(release, debug|release) {
DEFINES += NDEBUG NVALGRIND DYNAMIC_ANNOTATIONS_ENABLED=0

QMAKE_CXXFLAGS += -O2

QMAKE_LFLAGS += -Wl,-O1 -Wl,--as-needed -Wl,--gc-sections

LIBS += $$OUT_PWD/../lib/release/libdynamic_annotations.a \
    -L$$PWD/../lib/release -lbase \
    -L$$PWD/../lib/release -licuuc -licui18n

DESTDIR = $$PWD/../lib/release/
}

SOURCES += \
    gurl.cc \
    origin.cc \
    scheme_host_port.cc \
    third_party/mozilla/url_parse.cc \
    url_canon_etc.cc \
    url_canon_filesystemurl.cc \
    url_canon_fileurl.cc \
    url_canon_host.cc \
    url_canon_icu.cc \
    url_canon_internal.cc \
    url_canon_ip.cc \
    url_canon_mailtourl.cc \
    url_canon_path.cc \
    url_canon_pathurl.cc \
    url_canon_query.cc \
    url_canon_relative.cc \
    url_canon_stdstring.cc \
    url_canon_stdurl.cc \
    url_constants.cc \
    url_parse_file.cc \
    url_util.cc

HEADERS += \
    gurl.h \
    origin.h \
    scheme_host_port.h \
    third_party/mozilla/url_parse.h \
    url_canon.h \
    url_canon_icu.h \
    url_canon_internal.h \
    url_canon_ip.h \
    url_canon_stdstring.h \
    url_constants.h \
    url_util.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
