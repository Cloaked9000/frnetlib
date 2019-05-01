//
// Created by fred on 06/12/16.
//

#ifndef FRNETLIB_SOCKET_H
#define FRNETLIB_SOCKET_H

#include <mutex>
#include "NetworkEncoding.h"
#include "SocketDescriptor.h"

#define RECV_CHUNK_SIZE 4096 //How much data to try and recv at once
namespace fr
{
    class Packet;
    class Sendable;
    class Socket : public SocketDescriptor
    {
    public:
        enum class Status
        {
            Unknown = 0,
            Success = 1,
            ListenFailed = 2,
            BindFailed = 3,
            Disconnected = 4,
            Error = 5,
            WouldBlock = 6,
            ConnectionFailed = 7,
            HandshakeFailed = 8,
            VerificationFailed = 9,
            MaxPacketSizeExceeded = 10,
            NotEnoughData = 11,
            ParseError = 12,
            HttpHeaderTooBig = 13,
            HttpBodyTooBig = 14,
            AddressLookupFailure = 15,
            SendError = 16,
            ReceiveError = 17,
            AcceptError = 18,
            SSLError = 19,
            NoRouteToHost = 20,
            Timeout = 21,
            //Remember to update status_to_string if more are added
        };

        enum class IP
        {
            v4 = 1,
            v6 = 2,
            any = 3
        };

        Socket();
        Socket(Socket &&) =delete;
        Socket(const Socket &) =delete;
        void operator=(const Socket &) =delete;
        void operator=(Socket &&) =delete;

        /*!
         * Converts an fr::Socket::Status value to a printable string
         *
         * Throws an std::logic_error if status is out of range.
         *
         * @note This should be called immediately after the error, as errno is used to help generate the string.
         * @param status Status value to convert
         * @return A string form version
         */
        static std::string status_to_string(fr::Socket::Status status);

        /*!
         * Connects the socket to an address.
         *
         * @param address The address of the socket to connect to
         * @param port The port of the socket to connect to
         * @param timeout The number of seconds to wait before timing the connection attempt out. Pass {} for default.
         * @return A Socket::Status indicating the status of the operation. (Success on success, an error type on failure).
         */
        virtual Socket::Status connect(const std::string &address, const std::string &port, std::chrono::seconds timeout)=0;


        /*!
         * Sets the socket to blocking or non-blocking.
         *
         * @note This must be set *WHILST* connected
         * @param should_block True for blocking (default argument), false otherwise.
         * @return Status of the operation:
         * 'Success' on success.
         * Something else (depending on underlying socket type) on failure.
         */
        virtual fr::Socket::Status set_blocking(bool should_block) = 0;

        /*!
         * Checks if the socket is blocking
         *
         * @return True if it is, false otherwise
         */
        virtual bool get_blocking() const=0;

        /*!
         * Attempts to send raw data down the socket, without
         * any of frnetlib's framing. Useful for communicating through
         * different protocols.
         *
         * @param data The data to send.
         * @param size The number of bytes, from data to send. Be careful not to overflow.
         * @param sent The number of bytes that could be sent.
         * @return The status of the operation. Dependent on the underlying socket type.
         */
        virtual Status send_raw(const char *data, size_t size, size_t &sent) = 0;

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
         * @return The status of the operation, if the socket has disconnected etc. This is dependent on the underlying socket type.
         */
        virtual Status receive_raw(void *data, size_t data_size, size_t &received) = 0;

        /*!
         * Sets the socket file descriptor. Internally used.
         *
         * @param descriptor_data The socket descriptor data, set up by the Listener.
         */
        virtual void set_descriptor(void *descriptor_data)=0;

        /*!
         * Send a Sendable object through the socket
         *
         * @param obj The object to send
         * @return The status of the send. This is dependant type being sent.
         */
        virtual Status send(const Sendable &obj);

        /*!
         * Receive a Sendable object through the socket
         *
         * @param obj The object to receive
         * @return The status of the receive
         * 'Disconnected' if the socket disconnected
         * 'Success' if the object could be read successfully
         * 'WouldBlock' if the socket is in blocking mode and no data could be read
         */
        virtual Status receive(Sendable &obj);

        /*!
         * Reads size bytes into dest from the socket.
         * Unlike receive_raw, this will keep trying
         * to receive data until 'size' bytes have been
         * read, or the client has disconnected/there was
         * an error.
         *
         * @param dest Where to read the data into
         * @param buffer_size The number of bytes to read
         * @return Status indicating if the send succeeded or not:
         * 'Success': All good, object still valid.
         * 'WouldBlock' or 'Timeout': No data received. Object still valid though.
         * Anything else: Object invalid. Call disconnect().
         */
        Socket::Status receive_all(void *dest, size_t buffer_size);

        /*!
         * Calls the shutdown syscall on the socket.
         * So you can receive data but not send.
         *
         * This can be called on a blocking socket to force
         * it to immediately return (you might want to do this if
         * you're exiting and need the blocking socket to return).
         */
        void shutdown();

        /*!
         * Set which IP version to use. IP::any is the default
         * value, so either an IPv4 OR IPv6 interface will be used.
         *
         * @param version Should IPv4, IPv6 be used, or any?
         */
        void set_inet_version(IP version);

        /*!
         * Ends, and closes the connection.
         * There is a distinction between 'disconnect' and 'close_socket',
         * in that 'disconnect' should end the connection properly (such as sending
         * disconnect packets depending on the protocol), before calling 'close_socket' itself.
         * 'close_socket' should just close the client connection and be done with it.
         */
        virtual void disconnect();

        /*!
         * Sets the maximum receivable size that may be received by the socket. This does
         * not apply to receive_raw(), but only things like fr::Packet.
         *
         * If a client attempts to send a packet larger than sz bytes, then
         * the client will be disconnected and an fr::Socket::MaxPacketSizeExceeded
         * will be returned. Pass '0' to indicate no limit.
         *
         * This should be used to prevent potential abuse, as a client could say that
         * it's going to send a 200GiB packet, which would cause the Socket to try and
         * allocate that much memory to accommodate the data, which is most likely not
         * desirable.
         *
         * By default, there is no limit (0)
         *
         * @param sz The maximum number of bytes that may be received in an fr::Packet
         */
        inline void set_max_receive_size(uint32_t sz)
        {
            max_receive_size = sz;
        }

        /*!
          * Sets a timeout which applies when receiving data.
          *
          * @note When receiving framed data, such as with receive(), this timeout will apply to the underlying
          * individual reads, but not for the message as a whole.
          *
          * @param timeout The maximum number of milliseconds to wait on a socket read before returning. Pass
          * 0 (default) for no timeout.
          */
        inline void set_receive_timeout(uint32_t timeout)
        {
            socket_read_timeout = timeout;
            reconfigure_socket();
        }

        /*!
         * Gets the socket receive timeout.
         *
         * @return Socket timeout in milliseconds. 0 if none.
         */
        inline uint32_t get_receive_timeout() const
        {
            return socket_read_timeout;
        }

        /*!
          * Sets a timeout which applies when sending data.
          *
          * @param timeout The maximum number of milliseconds to wait on a socket write before returning. Pass
          * 0 (default) for no timeout.
          */
        inline void set_send_timeout(uint32_t timeout)
        {
            socket_write_timeout = timeout;
            reconfigure_socket();
        }

        /*!
         * Gets the socket send timeout.
         *
         * @return Socket send timeout in milliseconds. 0 if none.
         */
        inline uint32_t get_send_timeout() const
        {
            return socket_write_timeout;
        }

        /*!
         * Gets the max packet size. See set_max_packet_size
         * for more information. If this returns 0, then
         * there is no limit.
         *
         * @return The max packet size
         */
        inline uint32_t get_max_receive_size() const
        {
            return max_receive_size;
        }

        /*!
         * Gets the socket's printable remote address
         *
         * @return The string address
         */
        const std::string &get_remote_address() const
        {
            return fr_socket_remote_address;
        }

        /*!
         * Sets the connections remote address.
         *
         * @param addr The remote address to use
         */
        inline void set_remote_address(const std::string &addr)
        {
            fr_socket_remote_address = addr;
        }
    protected:

        /*!
         * Close the connection.
         */
        virtual void close_socket()=0;

        /*!
         * Applies requested socket options to the socket.
         * Should be called when a new socket is created.
         */
        virtual void reconfigure_socket()=0;

        std::string fr_socket_remote_address;
        int ai_family;
        uint32_t max_receive_size;
        uint32_t socket_read_timeout;
        uint32_t socket_write_timeout;
    };
}


#endif //FRNETLIB_SOCKET_H