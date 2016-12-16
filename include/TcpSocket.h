//
// Created by fred on 06/12/16.
//

#ifndef FRNETLIB_TCPSOCKET_H
#define FRNETLIB_TCPSOCKET_H

#include <memory>
#include "Socket.h"

namespace fr
{
#define RECV_CHUNK_SIZE 2048 //How much data to try and recv at once

class TcpSocket : public Socket
{
public:
    TcpSocket() noexcept;
    virtual ~TcpSocket() noexcept;
    TcpSocket(TcpSocket &&) noexcept = default;
    void operator=(const TcpSocket &other)=delete;

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
    virtual void set_descriptor(int descriptor);

    /*!
     * Attempts to send raw data down the socket, without
     * any of frnetlib's framing. Useful for communicating through
     * different protocols.
     *
     * @param data The data to send.
     * @param size The number of bytes, from data to send. Be careful not to overflow.
     * @return The status of the operation.
     */
    virtual Status send_raw(const char *data, size_t size) override;


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
    virtual Status receive_raw(void *data, size_t data_size, size_t &received) override;

    /*!
     * Sets the connections remote address.
     *
     * @param addr The remote address to use
     */
    void set_remote_address(const std::string &addr)
    {
        remote_address = addr;
    }

    /*!
     * Sets if the socket should be blocking or non-blocking.
     *
     * @param should_block True to block, false otherwise.
     */
    virtual void set_blocking(bool should_block) override;

    /*!
     * Gets the unerlying socket descriptor
     *
     * @return The socket descriptor
     */
    int32_t get_socket_descriptor() const override;

protected:
    std::string unprocessed_buffer;
    std::unique_ptr<char[]> recv_buffer;
    int32_t socket_descriptor;
};

}


#endif //FRNETLIB_TCPSOCKET_H
