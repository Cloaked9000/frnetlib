//
// Created by fred.nicolson on 01/10/18.
//

#include <frnetlib/TcpListener.h>
#include <iostream>
#include <frnetlib/SocketSelector.h>
#include <frnetlib/Http.h>
#include <frnetlib/HttpRequest.h>
#include <frnetlib/HttpResponse.h>
#include <thread>

int main()
{
    //Bind to port
    auto listener = std::make_shared<fr::TcpListener>();
    if(listener->listen("8080") != fr::Socket::Success)
    {
        std::cerr << "Failed to bind to port" << std::endl;
        return EXIT_FAILURE;
    }

    for(size_t a = 0; a < std::thread::hardware_concurrency(); ++a)
    {
        if(fork() <= 0)
            break;
    }

    //Setup our socket selector and add the listener to it
    fr::SocketSelector selector;
    selector.add(listener, nullptr);

    //Enter main server loop
    volatile bool running = true;
    while(running)
    {
        //Wait for a socket to be ready with no timeout
        auto ready_sockets = selector.wait();

        //Process each socket
        for(auto &ready_socket : ready_sockets)
        {
            //If this is the listener, then accept a new connection
            if(ready_socket.first->get_socket_descriptor() == listener->get_socket_descriptor())
            {
                auto client = std::make_shared<fr::TcpSocket>();
                if(listener->accept(*client) == fr::Socket::Success)
                {
                    //We've accepted the connection, so add it to the selector, associating a new fr::HttpRequest
                    //Object which will slowly be filled with the client's request. You could also pass a struct
                    //associated with the connection for easier state management.
                    selector.add(client, new fr::HttpRequest());
                    client->set_blocking(false);
                }
                break;
            }

            //Else, a socket is sending data/has disconnected, accept it
            auto client = std::static_pointer_cast<fr::Socket>(ready_socket.first); //fr::TcpSocket -> fr::Socket -> fr::SocketDescriptor
            auto partial_request = static_cast<fr::HttpRequest*>(ready_socket.second);

            char data[0x1000];
            size_t received = 0;
            auto recv_status = client->receive_raw(data, sizeof(data), received);
            if(recv_status == fr::Socket::Success)
            {
                //We receied data, so parse it using the partial HTTP request associated with this connection
                auto parse_status = partial_request->parse(data, received);
                if(parse_status == fr::Socket::Success)
                {
                    //The client has sent a full request, send them something back
                    //This could alternatively pass this connection/request off to a handler in another
                    //thread for further processing.
                    fr::HttpResponse response;
                    response.set_body("<h1>Hello World!</h1>");
                    client->send(response);
                }
                else if(parse_status == fr::Socket::NotEnoughData)
                {
                    //More data needed, wait for it
                }
                else
                {
                    //HTTP error, disconnect the client
                    delete partial_request;
                    selector.remove(client);
                    client->disconnect();
                }
            }
            else if(recv_status == fr::Socket::Disconnected)
            {
                //Free the allocated fr::HttpRequest. The socket is auto-removed from the selector as it notified us.
                delete partial_request;
            }
        }
    }
}