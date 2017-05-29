add_executable(simple_client tcpsocket_client.cpp)
target_link_libraries(simple_client frnetlib)

add_executable(simple_server tcpsocket_server.cpp)
target_link_libraries(simple_server frnetlib)

install(TARGETS simple_client simple_server
        DESTINATION "bin")
