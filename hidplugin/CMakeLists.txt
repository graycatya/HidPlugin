cmake_minimum_required(VERSION 3.1.4 FATAL_ERROR)

project(HidPlugin LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(../hidapi hidapi.out)

list(APPEND HIDPLUGIN_PUBLIC_HEADERS "hidapi_winapi.h")


set(HIDAPI_HIDTEST_TARGETS)
if(NOT WIN32 AND NOT APPLE AND CMAKE_SYSTEM_NAME MATCHES "Linux")
  if(TARGET hidapi::hidraw)
      add_library(${PROJECT_NAME} STATIC HidPlugin.cpp
                                        HidPlugin.h)
      target_link_libraries(${PROJECT_NAME} hidapi::hidraw)
      list(APPEND HIDAPI_HIDTEST_TARGETS hidtest_hidraw)
  endif()
  if(TARGET hidapi::libusb)
      add_library(${PROJECT_NAME} STATIC HidPlugin.cpp
                                        HidPlugin.h)
      target_compile_definitions(${PROJECT_NAME} PRIVATE USING_HIDAPI_LIBUSB)
      target_link_libraries(${PROJECT_NAME} hidapi::libusb)
      list(APPEND HIDAPI_HIDTEST_TARGETS hidtest_libusb)
  endif()
else()
    add_library(${PROJECT_NAME} STATIC HidPlugin.cpp
                                        HidPlugin.h)
  target_link_libraries(${PROJECT_NAME} hidapi::hidapi)
  list(APPEND HIDAPI_HIDTEST_TARGETS hidtest)
endif()


target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

