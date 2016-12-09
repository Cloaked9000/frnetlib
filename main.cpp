#include <iostream>
#include <Packet.h>
#include <TcpSocket.h>
#include <TcpListener.h>
#include <SocketSelector.h>
#include <thread>
#include <atomic>
#include <vector>

void server()
{
    fr::TcpListener listener;
    if(listener.listen("8081") != fr::Socket::Success)
    {
        std::cout << "Failed to listen to port" << std::endl;
        return;
    }

    fr::SocketSelector selector;
    std::vector<std::unique_ptr<fr::TcpSocket>> clients;

    selector.add(listener);

    while(selector.wait())
    {
        if(selector.is_ready(listener))
        {
            clients.emplace_back(new fr::TcpSocket());
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
                    fr::Packet packet;
                    if((*iter)->receive(packet) == fr::Socket::Success)
                    {
                        std::string message;
                        packet >> message;
                        std::cout << (*iter)->get_remote_address() << " sent: " << message << std::endl;
                        iter++;
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
    fr::TcpSocket socket;
    socket.connect("127.0.0.1", "8081");

    fr::TcpSocket socket2;
    socket2.connect("127.0.0.1", "8081");

    fr::Packet packet;
    packet << "Hello, World! - From socket 1";
    socket.send(packet);

    packet.clear();
    packet << "Hello, world! - From socket 2";
    socket2.send(packet);
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