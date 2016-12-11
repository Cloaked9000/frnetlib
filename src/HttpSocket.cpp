//
// Created by fred on 10/12/16.
//

#include <HttpResponse.h>
#include "HttpSocket.h"

namespace fr
{

    Socket::Status HttpSocket::receive(Http &request)
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
        request.parse(buffer);

        return Socket::Success;
    }

    Socket::Status HttpSocket::send(const Http &request)
    {
        std::string data = request.construct();
        return send_raw(&data[0], data.size());
    }
}