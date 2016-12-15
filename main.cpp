#include <iostream>
#include <SSLListener.h>
#include "include/Packet.h"
#include "include/TcpSocket.h"
#include "include/TcpListener.h"
#include "include/SocketSelector.h"
#include "HttpSocket.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "SSLSocket.h"

int main()
{
    fr::SSLListener listener;
    if(listener.listen("9091") != fr::Socket::Success)
    {
        std::cout << "Failed to bind to port" << std::endl;
        return 1;
    }

    while(true)
    {
        fr::HttpSocket<fr::SSLSocket> http_socket;
        if(listener.accept(http_socket) != fr::Socket::Success)
        {
            std::cout << "Failed to accept client" << std::endl;
            continue;
        }

        fr::HttpRequest request;
        if(http_socket.receive(request) != fr::Socket::Success)
        {
            std::cout << "Failed to receive data" << std::endl;
            continue;
        }
        else
        {
            std::cout << "Read successfully" << std::endl;
        }

        std::cout << "Got: " << request.get_body() << std::endl;

        fr::HttpResponse response;
        response.set_body("<h1>Hello, SSL World!</h1>");
        http_socket.send(response);
        http_socket.close();
    }


//    fr::SSLSocket socket;
//    if(socket.connect("lloydsenpai.xyz", "443") != fr::Socket::Success)
//        return 1;
//
//    std::string request = "GET / HTTP/1.1\r\nhost: www.lloydsenpai.xyz\r\n\r\n";
//    socket.send_raw(request.c_str(), request.size());
//
//    char *data = new char[1024];
//    size_t received;
//    if(socket.receive_raw(data, 1024, received) != fr::Socket::Success)
//        return 2;
//
//    std::cout << "Got: " << std::string(data, received) << std::endl;

    return 0;
}