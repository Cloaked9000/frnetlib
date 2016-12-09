//
// Created by fred on 06/12/16.
//

#ifndef FRNETLIB_TCPLISTENER_H
#define FRNETLIB_TCPLISTENER_H
#include <string>
#include <netdb.h>
#include "TcpSocket.h"
#include "Socket.h"
#include "NetworkEncoding.h"

namespace fr
{

class TcpListener
{
public:
    /*!
     * Listens to the given port for connections
     *
     * @param port The port to bind to
     * @return If the operation was successful
     */
    Socket::Status listen(const std::string &port);

    /*!
     * Accepts a new connection.
     *
     * @param client Where to store the connection information
     * @return True on success. False on failure.
     */
    Socket::Status accept(TcpSocket &client);

private:
    int socket_descriptor;
};

}


#endif //FRNETLIB_TCPLISTENER_H
