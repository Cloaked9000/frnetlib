cmake_minimum_required(VERSION 3.6)
project(frnetlib)

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} /home/fred/ClionProjects/frnetlib/cmake_modules)

FIND_PACKAGE(MBEDTLS)
INCLUDE_DIRECTORIES(${MBEDTLS_INCLUDE_DIR})

include_directories(include)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -m64 -fPIC -pthread -lmbedtls -lmbedx509 -lmbedcrypto")

set(SOURCE_FILES main.cpp src/TcpSocket.cpp include/TcpSocket.h src/TcpListener.cpp include/TcpListener.h src/Socket.cpp include/Socket.h src/Packet.cpp include/Packet.h include/NetworkEncoding.h src/SocketSelector.cpp include/SocketSelector.h src/HttpSocket.cpp include/HttpSocket.h src/HttpRequest.cpp include/HttpRequest.h src/HttpResponse.cpp include/HttpResponse.h src/Http.cpp include/Http.h include/SSLSocket.cpp include/SSLSocket.h include/SSLListener.cpp include/SSLListener.h)
add_executable(frnetlib ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(frnetlib ${MBEDTLS_LIBRARIES} -lmbedtls -lmbedx509 -lmbedcrypto)