//
// Created by fred on 06/12/16.
//

#ifndef FRNETLIB_TCPSOCKET_H
#define FRNETLIB_TCPSOCKET_H

#include <memory>
#include "Socket.h"

namespace fr
{
#define RECV_CHUNK_SIZE 1024 //How much data to try and recv at once

class TcpSocket : public Socket
{
public:
    TcpSocket() noexcept;
    virtual ~TcpSocket() noexcept;
    TcpSocket(TcpSocket &&) noexcept = default;
    void operator=(const TcpSocket &other)=delete;

    /*!
     * Send a packet through the socket
     *
     * @param packet The packet to send
     * @return True on success, false on failure.
     */
    virtual Status send(const Packet &packet);

    /*!
     * Receive a packet through the socket
     *
     * @param packet The packet to receive
     * @return True on success, false on failure.
     */
    virtual Status receive(Packet &packet);

    /*!
     * Close the connection.
     */
    virtual void close();

    /*!
     * Connects the socket to an address.
     *
     * @param address The address of the socket to connect to
     * @param port The port of the socket to connect to
     * @return A Socket::Status indicating the status of the operation.
     */
    virtual Socket::Status connect(const std::string &address, const std::string &port);

    /*!
     * Sets the socket file descriptor.
     *
     * @param descriptor The socket descriptor.
     */
    void set_descriptor(int descriptor);

    /*!
     * Checks to see if we're connected to a socket or not
     *
     * @return True if it's connected. False otherwise.
     */
    inline bool connected() const
    {
        return is_connected;
    }

    /*!
     * Attempts to send raw data down the socket, without
     * any of frnetlib's framing. Useful for communicating through
     * different protocols.
     *
     * @param data The data to send.
     * @param size The number of bytes, from data to send. Be careful not to overflow.
     * @return The status of the operation.
     */
    Status send_raw(const char *data, size_t size);


    /*!
     * Receives raw data from the socket, without any of
     * frnetlib's framing. Useful for communicating through
     * different protocols. This will attempt to read 'data_size'
     * bytes, but might not succeed. It'll return how many bytes were actually
     * read in 'received'.
     *
     * @param data Where to store the received data.
     * @param data_size The number of bytes to try and receive. Be sure that it's not larger than data.
     * @param received Will be filled with the number of bytes actually received, might be less than you requested.
     * @return The status of the operation, if the socket has disconnected etc.
     */
    Status receive_raw(void *data, size_t data_size, size_t &received);

private:
    /*!
     * Reads size bytes into dest from the socket.
     * Unlike receive_raw, this will keep trying
     * to receive data until 'size' bytes have been
     * read, or the client has disconnected/there was
     * an error.
     *
     * @param dest Where to read the data into
     * @param size The number of bytes to read
     * @return Operation status.
     */
    Status receive_all(void *dest, size_t size);

    std::string unprocessed_buffer;
    std::unique_ptr<char[]> recv_buffer;
    bool is_connected;
};

}


#endif //FRNETLIB_TCPSOCKET_H
