#include <iostream>
#include <frnetlib/SSLListener.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include "frnetlib/Packet.h"
#include "frnetlib/TcpSocket.h"
#include "frnetlib/TcpListener.h"
#include "frnetlib/SocketSelector.h"
#include "frnetlib/HttpSocket.h"
#include "frnetlib/HttpRequest.h"
#include "frnetlib/HttpResponse.h"
#include "frnetlib/SSLSocket.h"
#include "frnetlib/SSLContext.h"
#include "frnetlib/SSLListener.h"

void server()
{
    fr::TcpListener listener;
    listener.listen("9092");

    fr::TcpSocket socket;
    listener.accept(socket);

    uint64_t packet_count = 0;
    auto last_print_time = std::chrono::system_clock::now();
    while(true)
    {
        fr::Packet packet;
        if(socket.receive(packet) != fr::Socket::Success)
            break;


        std::string s1;
        packet >> s1;

        packet_count++;
        if(last_print_time + std::chrono::seconds(1) < std::chrono::system_clock::now())
        {
            std::cout << "Packets per second: " << packet_count << std::endl;
            packet_count = 0;
            last_print_time = std::chrono::system_clock::now();
        }
    }

}

int main()
{
    std::thread server_thread(server);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    fr::TcpSocket socket;
    socket.connect("127.0.0.1", "9092");

    std::string a(32384, 'c');
    fr::Packet packet;
    while(true)
    {
        packet << a;
        if(socket.send(packet) != fr::Socket::Success)
            break;
        packet.clear();
    }
    server_thread.join();
}