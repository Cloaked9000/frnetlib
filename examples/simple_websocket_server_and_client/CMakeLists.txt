add_executable(simple_websocket_client SimpleWebsocketClient.cpp)
target_link_libraries(simple_websocket_client frnetlib)

add_executable(simple_websocket_server SimpleWebsocketServer.cpp)
target_link_libraries(simple_websocket_server frnetlib)

install(TARGETS simple_websocket_client simple_websocket_server
        DESTINATION "bin")
