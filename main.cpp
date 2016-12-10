#include <iostream>
#include "include/Packet.h"
#include "include/TcpSocket.h"
#include "include/TcpListener.h"
#include "include/SocketSelector.h"
#include <thread>
#include <atomic>
#include <vector>
#include <HttpSocket.h>

void server()
{
    fr::TcpListener listener;
    if(listener.listen("8081") != fr::Socket::Success)
    {
        std::cout << "Failed to listen to port" << std::endl;
        return;
    }

    fr::SocketSelector selector;
    std::vector<std::unique_ptr<fr::HttpSocket>> clients;

    selector.add(listener);

    while(selector.wait())
    {
        if(selector.is_ready(listener))
        {
            clients.emplace_back(new fr::HttpSocket());
            if(listener.accept(*clients.back()) != fr::Socket::Success)
            {
                clients.pop_back();
                continue;
            }

            selector.add(*clients.back());
            std::cout << "Got new connection from: " << clients.back()->get_remote_address() << std::endl;
        }
        else
        {
            for(auto iter = clients.begin(); iter != clients.end();)
            {
                if(selector.is_ready(**iter))
                {
                    fr::HttpRequest request;
                    if((*iter)->receive(request) == fr::Socket::Success)
                    {
                        std::cout << "Requested: " << request.get_uri() << std::endl;
                        request.clear();
                        request.set_body("<h1>Hello, World!</h1>");

                        (*iter)->send(request);
                        (*iter)->close();
                    }
                    else
                    {
                        std::cout << (*iter)->get_remote_address() << " has disconnected." << std::endl;
                        selector.remove(*iter->get());
                        iter = clients.erase(iter);
                    }
                }
                else
                {
                    iter++;
                }
            }
        }
    }
}

void client()
{
//    fr::TcpSocket socket;
//    socket.connect("127.0.0.1", "8081");
//
//    fr::TcpSocket socket2;
//    socket2.connect("127.0.0.1", "8081");
//
//    fr::Packet packet;
//    packet << "Hello, World! - From socket 1";
//    socket.send(packet);
//
//    packet.clear();
//    packet << "Hello, world! - From socket 2";
//    socket2.send(packet);
    return;

}

int main()
{
    std::thread t1(&server);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto start = std::chrono::system_clock::now();
    std::thread t2(&client);

    t1.join();



    return 0;
}