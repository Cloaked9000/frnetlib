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

class TcpListener : public Socket
{
public:
    TcpListener() noexcept = default;
    virtual ~TcpListener() noexcept = default;
    TcpListener(TcpListener &&o) noexcept = default;

    /*!
     * Listens to the given port for connections
     *
     * @param port The port to bind to
     * @return If the operation was successful
     */
    virtual Socket::Status listen(const std::string &port);

    /*!
     * Accepts a new connection.
     *
     * @param client Where to store the connection information
     * @return True on success. False on failure.
     */
    virtual Socket::Status accept(TcpSocket &client);

private:
    //Stubs
    virtual Status send(const Packet &packet){return Socket::Error;}
    virtual Status receive(Packet &packet){return Socket::Error;}
    virtual void close(){}
    virtual Socket::Status connect(const std::string &address, const std::string &port){return Socket::Error;}
};

}


#endif //FRNETLIB_TCPLISTENER_H
