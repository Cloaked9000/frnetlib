#include <iostream>
#include <frnetlib/SSLListener.h>
#include <thread>
#include <atomic>
#include <mutex>
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
    fr::TcpSocket client;

    listener.listen("8081");
    listener.accept(client);

    uint32_t packet_no = 0;

    while(true)
    {
        fr::Packet packet;
        client.receive(packet);


        uint32_t num = 0;
        packet >> num;

        if(num != ++packet_no)
        {
            std::cout << "Packet mismatch. Expected " << packet_no + 1 << ". Got " << num << std::endl;
            return;
        }
    }
}

void client()
{
    fr::TcpSocket server;
    server.connect("127.0.0.1", "8081");

    uint32_t packet_no = 0;
    std::mutex m1;

    auto lam = [&]()
    {
        while(true)
        {
            m1.lock();
            fr::Packet packet;
            packet << ++packet_no;
            m1.unlock();

            server.send(packet);
        }
    };

    std::thread t1(lam);
    std::thread t2(lam);
    std::thread t3(lam);
    std::thread t4(lam);
    t1.join();
}

int main()
{
    std::thread s1(server);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::thread c1(client);

    s1.join();
    c1.join();
    return 0;
}