//
// Created by fred.nicolson on 01/02/18.
//

#include <iostream>
#include <frnetlib/TcpSocket.h>
#include <frnetlib/HttpRequest.h>
#include <frnetlib/URL.h>
#include <frnetlib/HttpResponse.h>

int main()
{
    //Get an address to query from stdin
    std::string url;
    std::cout << "Enter URL: ";
    std::cin >> url;

    //Parse it into something easy to use
    fr::URL parsed_url(url);
    if(parsed_url.get_port().empty())
    {
        std::cerr << "No schema or port specified. Unable to connect." << std::endl;
        return EXIT_FAILURE;
    }

    //Try to connect to the parsed address
    fr::TcpSocket socket;
    if(socket.connect(parsed_url.get_host(), parsed_url.get_port(), {}) != fr::Socket::Success)
    {
        std::cerr << "Failed to connect to the specified URL" << std::endl;
        return EXIT_FAILURE;
    }

    //Construct a request, requesting the user provided URI
    fr::HttpRequest request;
    request.set_uri(parsed_url.get_uri());
    if(socket.send(request) != fr::Socket::Success)
    {
        std::cerr << "Failed to send HTTP request" << std::endl;
        return EXIT_FAILURE;
    }

    //Now wait for a response
    fr::HttpResponse response;
    if(socket.receive(response) != fr::Socket::Success)
    {
        std::cerr << "Failed to receive HTTP response" << std::endl;
        return EXIT_FAILURE;
    }

    //Print out the response body
    std::cout << response.get_body() << std::endl;

}