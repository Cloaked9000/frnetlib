//
// Created by fred on 06/12/16.
//

#ifndef FRNETLIB_SOCKET_H
#define FRNETLIB_SOCKET_H


#include "NetworkEncoding.h"
#include "Packet.h"

namespace fr
{
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
        };

        Socket()
        : is_blocking(true)
        {

        }

        /*!
         * Send a packet through the socket
         *
         * @param packet The packet to send
         * @return True on success, false on failure.
         */
        virtual Status send(const Packet &packet)=0;

        /*!
         * Receive a packet through the socket
         *
         * @param packet The packet to receive
         * @return True on success, false on failure.
         */
        virtual Status receive(Packet &packet)=0;

        /*!
         * Close the connection.
         */
        virtual void close()=0;

        /*!
         * Connects the socket to an address.
         *
         * @param address The address of the socket to connect to
         * @param port The port of the socket to connect to
         * @return A Socket::Status indicating the status of the operation.
         */
        virtual Socket::Status connect(const std::string &address, const std::string &port)=0;

        /*!
         * Sets the socket's printable remote address
         *
         * @param addr The string address
         */
        inline virtual void set_remote_address(const std::string &addr)
        {
            remote_address = addr;
        }

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
         * Gets the socket descriptor of the object
         *
         * @return The socket file descriptor
         */
        inline int32_t get_socket_descriptor() const
        {
            return socket_descriptor;
        }

        /*!
         * Sets the socket to blocking or non-blocking.
         *
         * @param should_block True for blocking (default argument), false otherwise.
         */
        inline virtual void set_blocking(bool should_block = true)
        {
            //Don't update it if we're already in that mode
            if(should_block == is_blocking)
                return;

            //Different API calls needed for both windows and unix
            #ifdef WIN32
                u_long non_blocking = should_block ? 0 : 1;
                ioctlsocket(socket_descriptor, FIONBIO, &non_blocking);
            #else
                int flags = fcntl(socket_descriptor, F_GETFL, 0);
                fcntl(socket_descriptor, F_SETFL, is_blocking ? flags ^ O_NONBLOCK : flags ^= O_NONBLOCK);
            #endif

            is_blocking = should_block;
        }

    protected:
        int32_t socket_descriptor;
        std::string remote_address;
        bool is_blocking;
    };
}


#endif //FRNETLIB_SOCKET_H
