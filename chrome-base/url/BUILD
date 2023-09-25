licenses(["notice"])
package(default_visibility = ["//visibility:public"])
cc_library(name = "url",
    srcs = [
        "gurl.cc",
        "origin.cc",
        "scheme_host_port.cc",
        "third_party/mozilla/url_parse.cc",
        "url_canon_etc.cc",
        "url_canon_filesystemurl.cc",
        "url_canon_fileurl.cc",
        "url_canon_host.cc",
        "url_canon_icu.cc",
        "url_canon_internal.cc",
        "url_canon_ip.cc",
        "url_canon_mailtourl.cc",
        "url_canon_path.cc",
        "url_canon_pathurl.cc",
        "url_canon_query.cc",
        "url_canon_relative.cc",
        "url_canon_stdstring.cc",
        "url_canon_stdurl.cc",
        "url_constants.cc",
        "url_parse_file.cc",
        "url_util.cc",
    ],
    hdrs = [
        "gurl.h",
        "origin.h",
        "scheme_host_port.h",
        "third_party/mozilla/url_parse.h",
        "url_canon.h",
        "url_canon_icu.h",
        "url_canon_internal.h",
        "url_canon_ip.h",
        "url_canon_stdstring.h",
        "url_constants.h",
        "url_util.h",
    ],
    includes = [
        ".",
    ],
    copts = [
        "-Ithird_party/chromium/",
        "-Ithird_party/chromium/third_party/icu/source/common",
        "-I/usr/include/glib-2.0",
        "-fuse-ld=gold",
        "-std=gnu++11",
    ],
    deps = [
        "//third_party/chromium/base:base",
        "//third_party/chromium/base/third_party/dynamic_annotations:dynamic_annotations",
        "//third_party/chromium/third_party/icu/source:icu_debug",
    ],
)


cc_test(name = "at_exit_test",
    srcs = ["at_exit_unittest.cc"],
    deps = [
        ":base",
        "//third_party/gtest:gtest_main",
    ],
    linkopts = [
        "-lglib-2.0",
        "-ldl",
    ],
    copts = [
        "-Ithird_party/chromium/url",
        "-DUNIT_TEST",
    ],   
)
