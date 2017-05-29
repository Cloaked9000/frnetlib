cmake_minimum_required(VERSION 3.5)
project(frnetlib_test)

if( WIN32 )
    set( ADDITIONAL_LIB ws2_32 )  # Ws2_32.lib

	set( FRNETLIB_ROOT_PATH "C:/tools/cmake_install_libs/frnetlib/" ) # change it to your install directory

	set( FRNETLIB_INCLUDE_PATH ${FRNETLIB_ROOT_PATH}/include )
	set( FRNETLIB_LIB ${FRNETLIB_ROOT_PATH}/lib/frnetlib-s-d.lib )

	include_directories( ${FRNETLIB_INCLUDE_PATH} )
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -std=c++14")

elseif(APPLE)
	set( ADDITIONAL_LIB "" )
	set( FRNETLIB_LIB frnetlib)
else()
    set( ADDITIONAL_LIB "" )
	set( FRNETLIB_LIB frnetlib)
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -m64 -fPIC -std=c++14 -pthread -lmbedtls -lmbedx509 -lmbedcrypto")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -m64 -fPIC -std=c++14 -pthread")
endif()


add_executable(${PROJECT_NAME} ../main.cpp)
add_executable(packet packet.cpp)
add_executable(tcpsocket_server tcpsocket_server.cpp)
add_executable(tcpsocket_client tcpsocket_client.cpp)

target_link_libraries(${PROJECT_NAME} ${FRNETLIB_LIB} ${ADDITIONAL_LIB})
target_link_libraries(packet ${FRNETLIB_LIB} ${ADDITIONAL_LIB})
target_link_libraries(tcpsocket_server ${FRNETLIB_LIB} ${ADDITIONAL_LIB})
target_link_libraries(tcpsocket_client ${FRNETLIB_LIB} ${ADDITIONAL_LIB})