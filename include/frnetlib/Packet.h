//
// Created by fred on 06/12/16.
//

#ifndef FRNETLIB_PACKET_H
#define FRNETLIB_PACKET_H
#include <string>
#include <cstring>
#include <iostream>
#include "NetworkEncoding.h"

namespace fr
{
    class Packet
    {
    public:
        Packet() noexcept = default;

        //Nasty constructor to allow things like Packet{1, 2, 3, "bob"}.
        template <typename T, typename ...Args>
        Packet(T const &part, Args &&...args)
        {
            add(part, std::forward<Args>(args)...);
        }

        template<typename T, typename ...Args>
        void add(T const& part, Args &&...args)
        {
            *this << part;
            add(std::forward<Args>(args)...);
        }

        template<typename T>
        void add(T const part)
        {
            *this << part;
        }

        /*!
         * Gets the data added to the packet
         *
         * @return A string containing all of the data added to the packet
         */
        inline const std::string &get_buffer() const
        {
            return buffer;
        }

        /*
         * Adds a boolean variable to the packet
         */
        inline Packet &operator<<(bool var)
        {
            buffer.resize(buffer.size() + sizeof(var));
            memcpy(&buffer[buffer.size() - sizeof(var)], &var, sizeof(var));
            return *this;
        }

        /*
         * Extracts a boolean variable from the packet
         */
        inline Packet &operator>>(bool &var)
        {
            memcpy(&var, &buffer[0], sizeof(var));
            buffer.erase(0, sizeof(var));
            return *this;
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

        /*
         * Adds a string variable to the packet
         */
        inline Packet &operator<<(const std::string &var)
        {
            //Strings are prefixed with their length as a 32bit uint :)
            *this << (uint32_t)var.length();
            buffer += var;
            return *this;
        }

        /*
         * Adds a char array
         */
        inline Packet &operator<<(const char *var)
        {
            *this << std::string(var);
            return *this;
        }

        /*
         * Removes a string variable from the packet
         */
        inline Packet&operator>>(std::string &var)
        {
            uint32_t length = (uint32_t)var.length();
            *this >> length;

            var = buffer.substr(0, length);
            buffer.erase(0, length);

            return *this;
        }

        /*!
         * Sets the internal data buffer
         *
         * @param data What to set the packet to
         */
        inline void set_buffer(std::string &&data)
        {
            buffer = std::move(data);
        }

        /*!
         * Clears all data from the packet
         */
        inline void clear()
        {
            buffer.clear();
        }

    private:
        std::string buffer; //Packet data buffer
    };
}


#endif //FRNETLIB_PACKET_H
