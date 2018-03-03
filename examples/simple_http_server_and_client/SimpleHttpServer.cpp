//
// Created by fred.nicolson on 01/02/18.
//

#include <frnetlib/HttpRequest.h>
#include <frnetlib/HttpResponse.h>
#include <frnetlib/TcpListener.h>
#include <iostream>

int main()
{
    fr::Socket::Status  err;
    fr::TcpSocket client;
    fr::TcpListener listener;

    //Bind to a port
    if((err = listener.listen("8081")) != fr::Socket::Success)
    {
        std::cerr << "Failed to bind to port: " << err << std::endl;
        return EXIT_FAILURE;
    }

    while(true)
    {
        //Accept a new connection
        if((err = listener.accept(client)) != fr::Socket::Success)
        {
            std::cerr << "Failed to accept new connection: " << err << std::endl;
            continue;
        }

        //Receive client HTTP request
        fr::HttpRequest request;
        if((err = client.receive(request)) != fr::Socket::Success)
        {
            std::cerr << "Failed to receive request from client: " << err << std::endl;
        }

        //Construct a response
        fr::HttpResponse response;
        response.set_body("<h1>Hello, World!</h1>");

        //Send it
        if((err = client.send(response)) != fr::Socket::Success)
        {
            std::cerr << "Failed to send response to client: " << err << std::endl;
        }

        //Close connection
        client.disconnect();
    }
}