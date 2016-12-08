//
// Created by fred on 06/12/16.
//

#ifndef FRNETLIB_TCPSOCKET_H
#define FRNETLIB_TCPSOCKET_H

#include "Socket.h"

namespace fr
{

class TcpSocket : public Socket
{
public:
    /*!
     * Send a packet through the socket
     *
     * @param packet The packet to send
     * @return True on success, false on failure.
     */
    virtual bool send(const Packet &packet);

    /*!
     * Receive a packet through the socket
     *
     * @param packet The packet to receive
     * @return True on success, false on failure.
     */
    virtual bool receive(Packet &packet);

    /*!
     * Close the connection.
     */
    virtual void close();

    /*!
     * Sets the socket file descriptor.
     *
     * @param descriptor The socket descriptor.
     */
    void set_descriptor(int descriptor);

private:
    ssize_t read_recv();
    std::string unprocessed_buffer;
};

}


#endif //FRNETLIB_TCPSOCKET_H
