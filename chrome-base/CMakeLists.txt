cmake_minimum_required(VERSION 3.10)

project(chrome-base)

add_subdirectory(base)

target_include_directories( base PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>)

add_subdirectory(base/third_party/dynamic_annotations)
add_subdirectory(base/third_party/symbolize)
add_subdirectory(base/third_party/xdg_mime)
add_subdirectory(base/third_party/libevent)
add_subdirectory(base/i18n)
add_subdirectory(third_party)
add_subdirectory(url)