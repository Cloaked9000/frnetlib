//
// Created by fred on 06/12/16.
//

#ifndef FRNETLIB_SOCKET_H
#define FRNETLIB_SOCKET_H

#include <mutex>
#include "NetworkEncoding.h"

#define RECV_CHUNK_SIZE 4096 //How much data to try and recv at once
namespace fr
{
    class Packet;
    class Sendable;
    class Socket
    {
    public:
        enum Status
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
            NotEnoughData = 11
        };

        enum IP
        {
            v4 = 1,
            v6 = 2,
            any = 3
        };

        Socket() noexcept;
        virtual ~Socket() noexcept = default;
        Socket(Socket &&o) noexcept
        {
            outbound_mutex.lock();
            inbound_mutex.lock();
            o.inbound_mutex.lock();
            o.outbound_mutex.lock();

            remote_address = std::move(o.remote_address);
            is_blocking = o.is_blocking;
            ai_family = o.ai_family;
            max_receive_size = o.max_receive_size;

            outbound_mutex.unlock();
            inbound_mutex.unlock();
            o.inbound_mutex.unlock();
            o.outbound_mutex.unlock();
        }

        /*!
         * Close the connection.
         */
        virtual void close_socket()=0;

        /*!
         * Connects the socket to an address.
         *
         * @param address The address of the socket to connect to
         * @param port The port of the socket to connect to
         * @return A Socket::Status indicating the status of the operation.
         */
        virtual Socket::Status connect(const std::string &address, const std::string &port)=0;

        /*!
         * Gets the socket's printable remote address
         *
         * @return The string address
         */
        inline const std::string &get_remote_address()
        {
            return remote_address;
        }

        /*!
         * Sets the socket to blocking or non-blocking.
         *
         * @param should_block True for blocking (default argument), false otherwise.
         */
        virtual void set_blocking(bool should_block) = 0;

        /*!
         * Attempts to send raw data down the socket, without
         * any of frnetlib's framing. Useful for communicating through
         * different protocols.
         *
         * @param data The data to send.
         * @param size The number of bytes, from data to send. Be careful not to overflow.
         * @return The status of the operation.
         */
        virtual Status send_raw(const char *data, size_t size) = 0;


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
        virtual Status receive_raw(void *data, size_t data_size, size_t &received) = 0;

        /*!
         * Gets the socket descriptor.
         *
         * @return The socket descriptor.
         */
        virtual int32_t get_socket_descriptor() const = 0;

        /*!
         * Checks to see if we're connected to a socket or not
         *
         * @return True if it's connected. False otherwise.
         */
        virtual bool connected() const =0;

        /*!
         * Send a Sendable object through the socket
         *
         * @param obj The object to send
         * @return The status of the send
         */
        Status send(Sendable &obj);
        Status send(Sendable &&obj);

        /*!
         * Receive a Sendable object through the socket
         *
         * @param obj The object to receive
         * @return The status of the receive
         */
        Status receive(Sendable &obj);

        /*!
         * Reads size bytes into dest from the socket.
         * Unlike receive_raw, this will keep trying
         * to receive data until 'size' bytes have been
         * read, or the client has disconnected/there was
         * an error.
         *
         * @param dest Where to read the data into
         * @param buffer_size The number of bytes to read
         * @return Operation status.
         */
        Status receive_all(void *dest, size_t buffer_size);

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
         * Sets the maximum receivable size that may be received by the socket. This does
         * not apply to receive_raw(), but only things like fr::Packet, or HTTP responses.
         *
         * If a client attempts to send a packet larger than sz bytes, then
         * the client will be disconnected and an fr::Socket::MaxPacketSizeExceeded
         * will be returned. Pass '0' to indicate no limit. The default value is 0.
         *
         * This should be used to prevent potential abuse, as a client could say that
         * it's going to send a 200GiB packet, which would cause the Socket to try and
         * allocate that much memory to accommodate the data, which is most likely not
         * desirable.
         *
         * @param sz The maximum number of bytes that may be received in an fr::Packet
         */
        void set_max_receive_size(uint32_t sz);

        /*!
         * Gets the max packet size. See set_max_packet_size
         * for more information.
         *
         * @return The max packet size
         */
        inline uint32_t get_max_receive_size()
        {
            return max_receive_size;
        }
    protected:

        /*!
         * Applies requested socket options to the socket.
         * Should be called when a new socket is created.
         */
        void reconfigure_socket();

        std::string remote_address;
        bool is_blocking;
        std::mutex outbound_mutex;
        std::mutex inbound_mutex;
        int ai_family;
        uint32_t max_receive_size;
        #ifdef _WIN32
                static WSADATA wsaData;
        #endif // _WIN32
        static uint32_t instance_count;
    };
}


#endif //FRNETLIB_SOCKET_H
