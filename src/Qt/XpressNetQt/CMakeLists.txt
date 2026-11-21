# XpressNetQt: the Qt version of the XpressNet library (version 2.0, November 2026)
# Builds a static library, that the example programs (and your own program) link with:
#   add_subdirectory(<path>/Code/XpressNetQt XpressNetQt)
#   target_link_libraries(<your program> PRIVATE XpressNetQt)
# Requires Qt 6 (Core, Network, SerialPort) and a C++17 compiler.

cmake_minimum_required(VERSION 3.16)
project(XpressNetQt VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network SerialPort)

add_library(XpressNetQt STATIC
  XpressNet/XpressNet.h
  XpressNet/XpressNet.cpp
  XpressNet/XpressNetSend.h
  XpressNet/XpressNetSend.cpp
  XpressNet/XpressNetReceive.h
  XpressNet/XpressNetReceive.cpp
  XpressNet/XpressNetReceiveZ21.h
  XpressNet/XpressNetReceiveZ21.cpp
  XpressNet/XpressNetAccAuto.h
  XpressNet/XpressNetAccAuto.cpp
  XpressNet/XpressNetLogFile.h
  XpressNet/XpressNetLogFile.cpp
  Communication/TCPSocket.h
  Communication/TCPSocket.cpp
  Communication/UDPSocket.h
  Communication/UDPSocket.cpp
  Communication/SerialPort.h
  Communication/SerialPort.cpp
)

target_include_directories(XpressNetQt PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/XpressNet
  ${CMAKE_CURRENT_SOURCE_DIR}/Communication
)

target_link_libraries(XpressNetQt PUBLIC Qt6::Core Qt6::Network Qt6::SerialPort)
