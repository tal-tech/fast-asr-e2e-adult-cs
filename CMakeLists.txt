cmake_minimum_required(VERSION 3.10)

set(SERVICE_NAME cppservice)

project(${SERVICE_NAME})

set (CPP_SERVICE_MAJOR_VERSION 0)
set (CPP_SERVICE_MINOR_VERSION 8)
set (CPP_SERVICE_PATCH_VERSION 1)
set (CPP_SERVICE_VERSION ${CPP_SERVICE_MAJOR_VERSION}.${CPP_SERVICE_MINOR_VERSION}.${CPP_SERVICE_PATCH_VERSION})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/package")

######################################################################
#全局编译标志
######################################################################

SET (CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,$ORIGIN/../lib" )

SET (CMAKE_SHARED_LINKER_FLAGS
    "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-rpath,$ORIGIN/../lib" )

set(CMAKE_SKIP_BUILD_RPATH TRUE)

set(BUILD_SHARED_LIBS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC -fsanitize-address-use-after-scope")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=bounds-strict")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g")

######################################################################
#全局链接和包含
######################################################################
# 算法头文件
include_directories(${PROJECT_SOURCE_DIR}/alg/include)
link_directories(${PROJECT_SOURCE_DIR}/alg/lib)
link_directories(${PROJECT_SOURCE_DIR}/alg/lib/third_party)

#################################################################L#####
#全局编译输出指定
######################################################################

IF(CMAKE_BUILD_TYPE MATCHES Debug)
set(CMAKE_VERBOSE_MAKEFILE ON)
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE)


set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/${BUILD_TYPE})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${BUILD_TYPE})


######################################################################
#子工程目录
######################################################################


add_subdirectory(chrome-base)


######################################################################
#算法sdk编译对象
######################################################################



######################################################################
#编译对象
######################################################################

include_directories(${PROJECT_SOURCE_DIR}/chrome-base)

add_executable(main
    main.cpp)

target_link_libraries(
    main
    base
    pthread
    talparaformer
    )




######################################################################
#程序发布设置
######################################################################

install(DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ DESTINATION bin)
install(DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/ DESTINATION lib)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/alg/resource/ DESTINATION bin/res)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/alg/lib/ DESTINATION lib)
