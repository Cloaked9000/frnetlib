//
// Created by fred on 10/12/16.
//

#include "HttpSocket.h"

namespace fr
{

    Socket::Status HttpSocket::receive(HttpRequest &request)
    {
        //Create buffer to receive the request
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

    Socket::Status HttpSocket::send(const HttpRequest &request)
    {
        std::string data = request.get_request();
        return send_raw(&data[0], data.size());
    }
}