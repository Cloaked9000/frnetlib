#include <iostream>
#include <Packet.h>
#include <TcpSocket.h>
#include <TcpListener.h>
#include <thread>

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
        std::cout << "Got: " << message << std::endl;
    }
}

void client()
{
    fr::TcpSocket socket;
    socket.connect("127.0.0.1", "8081");

    fr::Packet packet;
    packet << "Hello, World!";
    socket.send(packet);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

int main()
{
    std::thread t1(&server);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    client();

    t1.join();
    return 0;
}