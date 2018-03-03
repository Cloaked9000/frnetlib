add_executable(simple_http_client SimpleHttpClient.cpp)
target_link_libraries(simple_http_client frnetlib)

add_executable(simple_http_server SimpleHttpServer.cpp)
target_link_libraries(simple_http_server frnetlib)

install(TARGETS simple_http_client simple_http_server
        DESTINATION "bin")
