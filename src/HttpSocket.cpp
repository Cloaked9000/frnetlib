//
// Created by fred on 10/12/16.
//

#include "HttpSocket.h"

namespace fr
{

    Socket::Status HttpSocket::receive_request(HttpRequest &request)
    {
        //Create buffer to receive_request the request
        std::string buffer(2048, '\0');

        //Receive the request
        size_t received;
        Socket::Status status = receive_raw(&buffer[0], buffer.size(), received);
        if(status != Socket::Success)
            return status;
        buffer.resize(received);

        //Parse it
        request.parse_request(buffer);

        return Socket::Success;
    }

    Socket::Status HttpSocket::receive_response(HttpRequest &response)
    {
        //Create buffer to receive_request the response
        std::string buffer(2048, '\0');

        //Receive the response
        size_t received;
        Socket::Status status = receive_raw(&buffer[0], buffer.size(), received);
        if(status != Socket::Success)
            return status;
        buffer.resize(received);

        //Parse it
        response.parse_response(buffer);

        return Socket::Success;
    }

    Socket::Status HttpSocket::send_request(const HttpRequest &request)
    {
        std::string data = request.construct_request();
        return send_raw(&data[0], data.size());
    }

    Socket::Status HttpSocket::send_response(const HttpRequest &request)
    {
        std::string data = request.construct_response();
        return send_raw(&data[0], data.size());
    }
}