cmake_minimum_required(VERSION 3.5)
project(frnetlib)

#Set module path
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules)

#User options
option(USE_SSL "Use SSL" ON)
set(FRNETLIB_BUILD_SHARED_LIBS true CACHE BOOL "Build shared library.")

if(USE_SSL)
    FIND_PACKAGE(MBEDTLS)
    INCLUDE_DIRECTORIES(${MBEDTLS_INCLUDE_DIR})
    add_definitions(-DSSL_ENABLED)
endif()

set( INCLUDE_PATH "${PROJECT_SOURCE_DIR}/include" )
set( SOURCE_PATH "${PROJECT_SOURCE_DIR}/src" )

set(SOURCE_FILES main.cpp src/TcpSocket.cpp include/frnetlib/TcpSocket.h src/TcpListener.cpp include/frnetlib/TcpListener.h src/Socket.cpp include/frnetlib/Socket.h src/Packet.cpp include/frnetlib/Packet.h include/frnetlib/NetworkEncoding.h src/SocketSelector.cpp include/frnetlib/SocketSelector.h src/HttpSocket.cpp include/frnetlib/HttpSocket.h src/HttpRequest.cpp include/frnetlib/HttpRequest.h src/HttpResponse.cpp include/frnetlib/HttpResponse.h src/Http.cpp include/frnetlib/Http.h src/SSLSocket.cpp include/frnetlib/SSLSocket.h src/SSLListener.cpp include/frnetlib/SSLListener.h include/frnetlib/SSLContext.h)

include_directories(include)

# Set the library output directory
set( LIBRARY_OUTPUT_PATH "${PROJECT_BINARY_DIR}/lib" )

# Add the library.
if( FRNETLIB_BUILD_SHARED_LIBS )
    add_library( frnetlib SHARED ${SOURCE_FILES} )
    set_target_properties( frnetlib PROPERTIES DEBUG_POSTFIX -d )
else()
    add_definitions( -DFRNETLIB_STATIC )
    add_library( frnetlib ${SOURCE_FILES} )

    set_target_properties( frnetlib PROPERTIES DEBUG_POSTFIX -s-d )
    set_target_properties( frnetlib PROPERTIES RELEASE_POSTFIX -s )
    set_target_properties( frnetlib PROPERTIES MINSIZEREL_POSTFIX -s )
endif()

# Tell the compiler to export when necessary.
set_target_properties( frnetlib PROPERTIES DEFINE_SYMBOL FRNETLIB_EXPORTS )

if( WIN32 )
    set( SHARE_PATH "." )
    set( LIB_PATH "lib" )
elseif(APPLE)
    set( SHARE_PATH "${CMAKE_INSTALL_PREFIX}/share/FRNETLIB" )
    set( LIB_PATH "lib" )
else()
    set( SHARE_PATH "${CMAKE_INSTALL_PREFIX}/share/FRNETLIB" )

    if( LIB_SUFFIX )
        set( LIB_PATH "lib${LIB_SUFFIX}" )
    else()
        set( LIB_PATH "lib" )
    endif()
endif()


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -m64 -fPIC")

if(USE_SSL)
    if( WIN32 )
        TARGET_LINK_LIBRARIES(frnetlib ${MBEDTLS_LIBRARIES} -lws2_32)
    else()
        TARGET_LINK_LIBRARIES(frnetlib ${MBEDTLS_LIBRARIES})
    endif()
endif()

install(
        TARGETS frnetlib
        RUNTIME DESTINATION bin COMPONENT bin
        LIBRARY DESTINATION "${LIB_PATH}" COMPONENT bin
        ARCHIVE DESTINATION "${LIB_PATH}" COMPONENT dev
)

install(
        DIRECTORY include
        DESTINATION .
)