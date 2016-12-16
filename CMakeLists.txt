cmake_minimum_required(VERSION 3.5)
project(frnetlib)

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules)
option(USE_SSL "Use SSL" ON)

if(USE_SSL)
    FIND_PACKAGE(MBEDTLS)
    INCLUDE_DIRECTORIES(${MBEDTLS_INCLUDE_DIR})
    add_definitions(-DSSL_ENABLED)
endif()

include_directories(include)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -m64 -fPIC -pthread -lmbedtls -lmbedx509 -lmbedcrypto")

set(SOURCE_FILES main.cpp src/TcpSocket.cpp include/TcpSocket.h src/TcpListener.cpp include/TcpListener.h src/Socket.cpp include/Socket.h src/Packet.cpp include/Packet.h include/NetworkEncoding.h src/SocketSelector.cpp include/SocketSelector.h src/HttpSocket.cpp include/HttpSocket.h src/HttpRequest.cpp include/HttpRequest.h src/HttpResponse.cpp include/HttpResponse.h src/Http.cpp include/Http.h src/SSLSocket.cpp include/SSLSocket.h src/SSLListener.cpp include/SSLListener.h include/SSLContext.h)
add_executable(frnetlib ${SOURCE_FILES})

if(USE_SSL)
    TARGET_LINK_LIBRARIES(frnetlib ${MBEDTLS_LIBRARIES} -lmbedtls -lmbedx509 -lmbedcrypto)
endif()