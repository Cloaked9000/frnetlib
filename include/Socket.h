//
// Created by fred on 06/12/16.
//

#ifndef FRNETLIB_SOCKET_H
#define FRNETLIB_SOCKET_H


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
        };

        Socket()
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
        inline virtual const std::string &get_remote_address()
        {
            return remote_address;
        }

    protected:
        int socket_descriptor;
        std::string remote_address;
    };
}


#endif //FRNETLIB_SOCKET_H
