#include <iostream>
#include "include/Packet.h"
#include "include/TcpSocket.h"
#include "include/TcpListener.h"
#include "include/SocketSelector.h"
#include <thread>
#include <atomic>
#include <vector>
#include "HttpSocket.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

void server()
{
    //Bind to port
    fr::TcpListener listener;
    if(listener.listen("8081") != fr::Socket::Success)
    {
        std::cout << "Failed to listen to port" << std::endl;
        return;
    }

    //Create a selector and a container for holding connected clients
    fr::SocketSelector selector;
    std::vector<std::unique_ptr<fr::HttpSocket>> clients;

    //Add our connection listener to the selector
    selector.add(listener);

    //Infinitely loop, waiting for connections or data
    while(selector.wait())
    {
        //If the listener is ready, that means we've got a new connection
        if(selector.is_ready(listener))
        {
            //Try and add them to our client container
            clients.emplace_back(new fr::HttpSocket());
            if(listener.accept(*clients.back()) != fr::Socket::Success)
            {
                clients.pop_back();
                continue;
            }

            //Add them to the selector if connected successfully
            selector.add(*clients.back());
            std::cout << "Got new connection from: " << clients.back()->get_remote_address() << std::endl;
        }
        else
        {
            //Else it's one of the clients who's sent some data. Check each one
            for(auto iter = clients.begin(); iter != clients.end();)
            {
                if(selector.is_ready(**iter))

                {
                    //This client has sent a HTTP request, so receive_request it
                    fr::HttpRequest request;
                    if((*iter)->receive(request) == fr::Socket::Success)
                    {
                        //Print to the console what we've been requested for
                        std::cout << "Requested: " << request.get_uri() << std::endl;

                        //Construct a response
                        fr::HttpResponse response;
                        response.set_body("<h1>Hello, World!</h1>");

                        //Send the response, and close the connection
                        (*iter)->send(response);
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
    fr::HttpSocket socket;
    if(socket.connect("127.0.0.1", "8081") != fr::Socket::Success)
    {
        std::cout << "Failed to connect to web server!" << std::endl;
        return;
    }

    socket.set_blocking(false);
    socket.set_blocking(true);
    fr::HttpResponse response;
    fr::Socket::Status status = socket.receive(response);
    if(status != fr::Socket::Success)
    {
        if(status == fr::Socket::WouldBlock)
           std::cout << "WouldBlock" << std::endl;
        std::cout << "Failed to receive HTTP response from the server!" << std::endl;
    }

    std::cout << "Got page body: " << response.get_body() << std::endl;
    return;

}

int main()
{
    std::thread t1(&server);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto start = std::chrono::system_clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    client();

    t1.join();



    return 0;
}