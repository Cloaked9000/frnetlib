//
// Created by fred.nicolson on 01/10/18.
//

#ifndef FRNETLIB_SOCKETDESCRIPTOR_H
#define FRNETLIB_SOCKETDESCRIPTOR_H

#include <stdint.h>
namespace fr
{
class SocketDescriptor
{
public:

    virtual ~SocketDescriptor()=default;

    /*!
     * Checks to see if the socket is connected or not
     *
     * @return True if connected, false otherwise
     */
    virtual bool connected() const = 0;

    /*!
     * Gets the underlying socket descriptor.
     *
     * @return The socket descriptor.
     */
    virtual int32_t get_socket_descriptor() const = 0;
};
}


#endif //FRNETLIB_SOCKETDESCRIPTOR_H
