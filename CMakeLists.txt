cmake_minimum_required(VERSION 3.6)
project(frnetlib)

include_directories(include)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -m64 -fPIC -pthread")

set(SOURCE_FILES main.cpp src/TcpSocket.cpp include/TcpSocket.h src/TcpListener.cpp include/TcpListener.h src/Socket.cpp include/Socket.h src/Packet.cpp include/Packet.h include/NetworkEncoding.h src/SocketSelector.cpp include/SocketSelector.h src/HttpSocket.cpp include/HttpSocket.h src/HttpRequest.cpp include/HttpRequest.h src/HttpResponse.cpp include/HttpResponse.h src/Http.cpp include/Http.h)
add_executable(frnetlib ${SOURCE_FILES})