//
// Created by fred on 06/12/16.
//

#ifndef FRNETLIB_TCPLISTENER_H
#define FRNETLIB_TCPLISTENER_H
#include <string>
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
    int32_t socket_descriptor;

    //Stubs
    virtual void close_socket(){}
    virtual Socket::Status connect(const std::string &address, const std::string &port){return Socket::Error;}
    virtual void set_blocking(bool val){}
    virtual fr::Socket::Status send_raw(const char*, size_t){return Socket::Error;}
    virtual fr::Socket::Status receive_raw(void*, size_t, size_t&){return Socket::Error;}
    virtual int32_t get_socket_descriptor() const {return socket_descriptor;}
};

}


#endif //FRNETLIB_TCPLISTENER_H
