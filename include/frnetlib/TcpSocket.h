//
// Created by fred on 06/12/16.
//

#ifndef FRNETLIB_TCPSOCKET_H
#define FRNETLIB_TCPSOCKET_H

#include <memory>
#include <mutex>
#include "Socket.h"

namespace fr
{
class TcpSocket : public Socket
{
public:
    TcpSocket() noexcept;
    ~TcpSocket() override;
    TcpSocket(TcpSocket &&) = delete;
    TcpSocket(const TcpSocket &) = delete;
    void operator=(TcpSocket &&)=delete;
    void operator=(const TcpSocket &)=delete;

    /*!
     * Connects the socket to an address.
     *
     * @param address The address of the socket to connect to
     * @param port The port of the socket to connect to
     * @param timeout The number of seconds to wait before timing the connection attempt out. Pass {} for default.
     * @return A Socket::Status indicating the status of the operation. (Success on success, an error type on failure).
     */
    Socket::Status connect(const std::string &address, const std::string &port, std::chrono::seconds timeout) override;

    /*!
     * Attempts to send raw data down the socket, without
     * any of frnetlib's framing. Useful for communicating through
     * different protocols.
     *
     * @param data The data to send.
     * @param size The number of bytes, from data to send. Be careful not to overflow.
     * @return The status of the operation.
     */
    Status send_raw(const char *data, size_t size) override;


    /*!
     * Receives raw data from the socket, without any of
     * frnetlib's framing. Useful for communicating through
     * different protocols. This will attempt to read 'data_size'
     * bytes, but might not succeed. It'll return how many bytes were actually
     * read in 'received'.
     *
     * @param data Where to store the received data.
     * @param buffer_size The number of bytes to try and receive. Be sure that it's not larger than data.
     * @param received Will be filled with the number of bytes actually received, might be less than you requested.
     * @return The status of the operation:
     * 'WouldBlock' if no data has been received, and the socket is in non-blocking mode or operation has timed out
     * 'Disconnected' if the socket has disconnected.
     * 'Success' All the bytes you wanted have been read
     */
    Status receive_raw(void *data, size_t buffer_size, size_t &received) override;

    /*!
     * Sets if the socket should be blocking or non-blocking.
     *
     * @note This must be set *WHILST* connected
     * @param should_block True to block, false otherwise.
     */
    void set_blocking(bool should_block) override;

    /*!
     * Sets the socket file descriptor. Internally used.
     *
     * @note For TcpSocket, this should be a pointer to a int32_t. A copy is made.
     * @param descriptor_data The socket descriptor data, set up by the Listener.
     */
    void set_descriptor(void *descriptor_data) override;

    /*!
     * Applies requested socket options to the socket.
     * Should be called when a new socket is created.
     */
    void reconfigure_socket() override;

    /*!
     * Checks to see if the socket is connected or not
     *
     * @return True if connected, false otherwise
     */
    bool connected() const noexcept override;

    /*!
     * Gets the underlying socket descriptor.
     *
     * @return The socket descriptor.
     */
    int32_t get_socket_descriptor() const noexcept override;

protected:

    /*!
     * Close the connection.
     */
    void close_socket() override;

    int32_t socket_descriptor;
};

}


#endif //FRNETLIB_TCPSOCKET_H
