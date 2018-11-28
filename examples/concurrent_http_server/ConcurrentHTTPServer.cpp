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

class SessionState
{
public:
    fr::HttpRequest partial_request;
};

void process_complete_request(const std::shared_ptr<fr::Socket> &client, fr::HttpRequest request)
{
    //Note: *NEVER* disconnect the client in the handler. Or it will never be removed from
    //the socket selector, and its opaque data will never be free'd. You're better off having a
    //disconnection queue which is processed by the listening thread, and added to here.
    fr::HttpResponse response;
    response.set_body("<h1>Hello World!</h1>");
    client->send(response);
}

int main()
{
    //Bind to port. Note that it is possible to fork/create new threads and then bind to the same
    //port multiple times. Each thread should have its own fr::SocketSelector, this will
    //spread connections over multiple workers.
    auto listener = std::make_shared<fr::TcpListener>();
    if(listener->listen("8080") != fr::Socket::Success)
    {
        std::cerr << "Failed to bind to port" << std::endl;
        return EXIT_FAILURE;
    }

    //Create a socket selector, and add the listener so we get notified on connection requests
    fr::SocketSelector listen_loop_selector;
    listen_loop_selector.add(listener, nullptr);

    bool running = true;
    while(running)
    {
        //Pass a timeout here, so that we can periodically check 'running' to see if we should exit
        auto ready_sockets = listen_loop_selector.wait(std::chrono::milliseconds(100));
        for(auto &ready_socket : ready_sockets)
        {
            //If it's the listener, accept a new connection
            if(ready_socket.first->get_socket_descriptor() == listener->get_socket_descriptor())
            {
                auto client = std::make_shared<fr::TcpSocket>(); //Or fr::SSLSocket
                if(listener->accept(*client) == fr::Socket::Success)
                {
                    client->set_blocking(false); //This is important
                    listen_loop_selector.add(client, new SessionState()); //We assign a 'SessionState' object for each connection as opaque data
                }
                continue;
            }

            //Else, we have new activity on a client socket
            auto client = std::static_pointer_cast<fr::Socket>(ready_socket.first);
            auto session = static_cast<SessionState*>(ready_socket.second);

            //Try and receive data
            char data[0x1000];
            size_t received = 0;
            auto recv_status = client->receive_raw(data, sizeof(data), received);
            if(recv_status == fr::Socket::Success)
            {
                //We received data, so parse it using the partial HTTP request associated with this connection
                auto parse_status = session->partial_request.parse(data, received);
                if(parse_status == fr::Socket::Success)
                {
                    //The client has sent a full request, queue it for processing
                    process_complete_request(client, std::move(session->partial_request));
                    session->partial_request = fr::HttpRequest();
                }
                else if(parse_status != fr::Socket::NotEnoughData)
                {
                    //HTTP error, disconnect the client. Remove from socket selector, and delete opaque data.
                    delete (SessionState*)listen_loop_selector.remove(client);
                }
            }
            else if(recv_status != fr::Socket::WouldBlock)
            {
                //Error, disconnect it. Remove from socket selector, and delete opaque data.
                delete (SessionState*)listen_loop_selector.remove(client);
            }
        }
    }
}