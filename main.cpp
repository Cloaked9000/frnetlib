#include <iostream>
#include <Packet.h>
#include <TcpSocket.h>
#include <TcpListener.h>
#include <thread>
#include <atomic>

std::atomic<uint64_t> data_sent;
void server()
{
    fr::TcpListener listener;
    listener.listen("8081");

    fr::TcpSocket socket;
    listener.accept(socket);

    while(socket.connected())
    {
        fr::Packet packet;
        socket.receive(packet);

        std::string message;
        packet >> message;
        data_sent += 0x100000;
    }
}

void client()
{
    fr::TcpSocket socket;
    socket.connect("127.0.0.1", "8081");

    std::string buffer(0x100000, 'c');

    fr::Packet packet;
    packet << buffer;
    while(true)
    {
        socket.send(packet);
    }
}

int main()
{
    data_sent = 0;
    std::thread t1(&server);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto start = std::chrono::system_clock::now();
    std::thread t2(&client);

    while(true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        uint64_t mb_sent = data_sent / 0x100000;
        std::cout << "Transfer speed: " << mb_sent / (uint64_t)std::chrono::duration<double>(std::chrono::system_clock::now() - start).count() << "MBps" << std::endl;
    }

    t1.join();
    return 0;
}