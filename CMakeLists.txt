cmake_minimum_required(VERSION 3.6)
project(frnetlib)

include_directories(include)
set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp src/TcpSocket.cpp include/TcpSocket.h src/TcpListener.cpp include/TcpListener.h src/Socket.cpp include/Socket.h src/Packet.cpp include/Packet.h include/NetworkEncoding.h)
add_executable(frnetlib ${SOURCE_FILES})