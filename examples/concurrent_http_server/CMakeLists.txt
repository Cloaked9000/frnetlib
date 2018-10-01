add_executable(concurrent_http_server ConcurrentHTTPServer.cpp)
target_link_libraries(concurrent_http_server frnetlib)

install(TARGETS concurrent_http_server DESTINATION "bin")
