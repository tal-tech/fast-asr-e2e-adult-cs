cmake_minimum_required(VERSION 3.3)
project(url)

include_directories(../
        ../third_party/icu/source/i18n
        ../third_party/icu/source/common)

add_definitions(
        -DUSE_SYMBOLIZE
        -DUSE_GLIB=1
        -D__STDC_CONSTANT_MACROS
        -D__STDC_FORMAT_MACROS
        -DICU_UTIL_DATA_IMPL=ICU_UTIL_DATA_FILE
        -DU_USING_ICU_NAMESPACE=0
        -DU_ENABLE_DYLOAD=0
        -DURL_IMPLEMENTATION)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector \
    --param=ssp-buffer-size=4 -pthread \
    -fno-strict-aliasing -Wall -Wextra -Wno-unused-parameter \
    -Wno-missing-field-initializers -fvisibility=hidden -pipe -fPIC \
    -fno-exceptions -fno-rtti -fno-threadsafe-statics \
    -fvisibility-inlines-hidden -std=gnu++11 \
    -m64 -march=x86-64")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g \
    -DCOMPONENT_BUILD -DDYNAMIC_ANNOTATIONS_ENABLED=1 \
    -D_DEBUG -D_GLIBCXX_DEBUG=1")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 \
    -DDYNAMIC_ANNOTATIONS_ENABLED=0 -DNDEBUG -DNVALGRIND")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} \
    -Wl,-z,now -Wl,-z,relro -Wl,--fatal-warnings -Wl,-z,defs -pthread \
    -Wl,-z,noexecstack -fPIC -fuse-ld=gold -m64 -Wl,--disable-new-dtags \
    -Wl,--detect-odr-violations -Wl,--icf=all")

set(SOURCE_FILES
        gurl.cc
        origin.cc
        scheme_host_port.cc
        third_party/mozilla/url_parse.cc
        url_canon_etc.cc
        url_canon_filesystemurl.cc
        url_canon_fileurl.cc
        url_canon_host.cc
        url_canon_icu.cc
        url_canon_internal.cc
        url_canon_ip.cc
        url_canon_mailtourl.cc
        url_canon_path.cc
        url_canon_pathurl.cc
        url_canon_query.cc
        url_canon_relative.cc
        url_canon_stdstring.cc
        url_canon_stdurl.cc
        url_constants.cc
        url_parse_file.cc
        url_util.cc
        gurl.h
        origin.h
        scheme_host_port.h
        third_party/mozilla/url_parse.h
        url_canon.h
        url_canon_icu.h
        url_canon_internal.h
        url_canon_ip.h
        url_canon_stdstring.h
        url_constants.h
        url_util.h)

IF(CMAKE_BUILD_TYPE MATCHES Debug)
    add_library(url SHARED ${SOURCE_FILES})
    target_link_libraries(url base icuuc icui18n rt dl)
ELSE(CMAKE_BUILD_TYPE MATCHES Debug)
    add_library(url STATIC ${SOURCE_FILES})
    add_dependencies(url
            base
            icuuc
            icui18n)
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)
