//
// Created by fred on 06/12/16.
//

#ifndef FRNETLIB_PACKET_H
#define FRNETLIB_PACKET_H
#include <string>
#include <netinet/in.h>
#include <cstring>
#include "NetworkEncoding.h"

class Packet
{
public:
    /*!
     * Gets the data added to the packet
     *
     * @return A string containing all of the data added to the packet
     */
    inline const std::string &construct_packet() const
    {
        return buffer;
    }

    /*
     * Adds a 16bit variable to the packet
     */
    inline Packet &operator<<(uint16_t var)
    {
        buffer.resize(buffer.size() + sizeof(var));
        var = htons(var);
        memcpy(&buffer[buffer.size() - sizeof(var)], &var, sizeof(var));
        return *this;
    }

    /*
     * Extracts a 16bit variable from the packet
     */
    inline Packet &operator>>(uint16_t &var)
    {
        memcpy(&var, &buffer[0], sizeof(var));
        buffer.erase(0, sizeof(var));
        var = ntohs(var);
        return *this;
    }

    /*
     * Adds a 32bit variable to the packet
     */
    inline Packet &operator<<(uint32_t var)
    {
        buffer.resize(buffer.size() + sizeof(var));
        var = htonl(var);
        memcpy(&buffer[buffer.size() - sizeof(var)], &var, sizeof(var));
        return *this;
    }

    /*
     * Extracts a 32bit variable from the packet
     */
    inline Packet &operator>>(uint32_t &var)
    {
        memcpy(&var, &buffer[0], sizeof(var));
        buffer.erase(0, sizeof(var));
        var = ntohl(var);
        return *this;
    }

    /*
     * Adds a 64bit variable to the packet
     */
    inline Packet &operator<<(uint64_t var)
    {
        buffer.resize(buffer.size() + sizeof(var));
        var = htonll(var);
        memcpy(&buffer[buffer.size() - sizeof(var)], &var, sizeof(var));
        return *this;
    }

    /*
     * Extracts a 64bit variable from the packet
     */
    inline Packet &operator>>(uint64_t &var)
    {
        memcpy(&var, &buffer[0], sizeof(var));
        buffer.erase(0, sizeof(var));
        var = ntohll(var);
        return *this;
    }

    /*
     * Adds a float variable to the packet
     */
    inline Packet &operator<<(float var)
    {
        buffer.resize(buffer.size() + sizeof(var));
        var = htonf(var);
        memcpy(&buffer[buffer.size() - sizeof(var)], &var, sizeof(var));
        return *this;
    }

    /*
     * Extracts a float variable from the packet
     */
    inline Packet &operator>>(float &var)
    {
        memcpy(&var, &buffer[0], sizeof(var));
        buffer.erase(0, sizeof(var));
        var = ntohf(var);
        return *this;
    }

    /*
     * Adds a double variable to the packet
     */
    inline Packet &operator<<(double var)
    {
        buffer.resize(buffer.size() + sizeof(var));
        var = htond(var);
        memcpy(&buffer[buffer.size() - sizeof(var)], &var, sizeof(var));
        return *this;
    }

    /*
     * Extracts a double variable from the packet
     */
    inline Packet &operator>>(double &var)
    {
        memcpy(&var, &buffer[0], sizeof(var));
        buffer.erase(0, sizeof(var));
        var = ntohd(var);
        return *this;
    }
private:
    std::string buffer; //Packet data buffer
};


#endif //FRNETLIB_PACKET_H
