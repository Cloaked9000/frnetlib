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
//    fr::SSLListener listener;
//    if(listener.listen("9091") != fr::Socket::Success)
//    {
//        std::cout << "Failed to bind to port" << std::endl;
//        return 1;
//    }
//
//    fr::SSLSocket socket;
//    if(listener.accept(socket) != fr::Socket::Success)
//    {
//        std::cout << "Failed to accept client" << std::endl;
//        return 2;
//    }
//
//    std::string buf(1024, '\0');
//    size_t received = 0;
//    if(socket.receive_raw(&buf[0], buf.size(), received) != fr::Socket::Success)
//    {
//        std::cout << "Failed to receive data" << std::endl;
//        return 3;
//    }
//
//    std::cout << "Got: " << buf.substr(0, received) << std::endl;
//
//
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