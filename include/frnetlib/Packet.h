//
// Created by fred on 06/12/16.
//

#ifndef FRNETLIB_PACKET_H
#define FRNETLIB_PACKET_H
#include <string>
#include <cstring>
#include <iostream>
#include <vector>
#include "NetworkEncoding.h"

namespace fr
{
    class Packet
    {
    public:
        Packet() noexcept
        : buffer_offset(0)
        {

        }

        //Nasty constructor to allow things like Packet{1, 2, 3, "bob"}.
        template <typename T, typename ...Args>
        Packet(T const &part, Args &&...args)
        {
            add(part, std::forward<Args>(args)...);
        }

        template<typename T, typename ...Args>
        inline void add(T const& part, Args &&...args)
        {
            *this << part;
            add(std::forward<Args>(args)...);
        }

        template<typename T>
        inline void add(T const part)
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
         * Adds a vector to a packet
         */
        template<typename T>
        inline Packet &operator<<(const std::vector<T> &vec)
        {
            //First store its length
            *this << vec.size();

            //Now each of the elements
            for(const auto &iter : vec)
            {
                *this << iter;
            }

            return *this;
        }

        /*
         * Extracts a vector from the packet
         */
        template<typename T>
        inline Packet &operator>>(std::vector<T> &vec)
        {
            size_t length;

            //First extract the length
            *this >> length;
            vec.resize(length);

            //Now take each of the elements out of the packet
            for(size_t a = 0; a < length; a++)
            {
                *this >> vec[a];
            }

            return *this;
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
            assert_data_remaining(sizeof(var));

            memcpy(&var, &buffer[buffer_offset], sizeof(var));
            buffer_offset += sizeof(var);
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
            assert_data_remaining(sizeof(var));

            memcpy(&var, &buffer[buffer_offset], sizeof(var));
            buffer_offset += sizeof(var);
            var = ntohs(var);
            return *this;
        }

        /*
         * Adds a 32bit variable to the packet
         */
        inline Packet &operator<<(uint32_t var)
        {
            var = htonl(var);
            buffer.resize(buffer.size() + sizeof(var));
            memcpy(&buffer[buffer.size() - sizeof(var)], &var, sizeof(var));
            return *this;
        }

        /*
         * Extracts a 32bit variable from the packet
         */
        inline Packet &operator>>(uint32_t &var)
        {
            assert_data_remaining(sizeof(var));

            memcpy(&var, &buffer[buffer_offset], sizeof(var));
            buffer_offset += sizeof(var);
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
            assert_data_remaining(sizeof(var));

            memcpy(&var, &buffer[buffer_offset], sizeof(var));
            buffer_offset += sizeof(var);
            var = ntohll(var);
            return *this;
        }

        /*
         * Adds a 16bit variable to the packet
         */
        inline Packet &operator<<(int16_t var)
        {
            buffer.resize(buffer.size() + sizeof(var));
            var = htons((uint16_t)var);
            memcpy(&buffer[buffer.size() - sizeof(var)], &var, sizeof(var));
            return *this;
        }

        /*
         * Extracts a 16bit variable from the packet
         */
        inline Packet &operator>>(int16_t &var)
        {
            assert_data_remaining(sizeof(var));

            memcpy(&var, &buffer[buffer_offset], sizeof(var));
            buffer_offset += sizeof(var);
            var = ntohs((uint16_t)var);
            return *this;
        }

        /*
         * Adds a 32bit variable to the packet
         */
        inline Packet &operator<<(int32_t var)
        {
            buffer.resize(buffer.size() + sizeof(var));
            var = htonl((uint32_t)var);
            memcpy(&buffer[buffer.size() - sizeof(var)], &var, sizeof(var));
            return *this;
        }

        /*
         * Extracts a 32bit variable from the packet
         */
        inline Packet &operator>>(int32_t &var)
        {
            assert_data_remaining(sizeof(var));

            memcpy(&var, &buffer[buffer_offset], sizeof(var));
            buffer_offset += sizeof(var);
            var = ntohl((uint32_t)var);
            return *this;
        }

        /*
         * Adds a 64bit variable to the packet
         */
        inline Packet &operator<<(int64_t var)
        {
            buffer.resize(buffer.size() + sizeof(var));
            var = htonll((uint64_t)var);
            memcpy(&buffer[buffer.size() - sizeof(var)], &var, sizeof(var));
            return *this;
        }

        /*
         * Extracts a 64bit variable from the packet
         */
        inline Packet &operator>>(int64_t &var)
        {
            assert_data_remaining(sizeof(var));

            memcpy(&var, &buffer[buffer_offset], sizeof(var));
            buffer_offset += sizeof(var);
            var = ntohll((uint64_t)var);
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
            assert_data_remaining(sizeof(var));

            memcpy(&var, &buffer[buffer_offset], sizeof(var));
            buffer_offset += sizeof(var);
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
            assert_data_remaining(sizeof(var));

            memcpy(&var, &buffer[buffer_offset], sizeof(var));
            buffer_offset += sizeof(var);;
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
            uint32_t length;
            *this >> length;

            var = buffer.substr(buffer_offset, length);
            buffer_offset += length;

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
            buffer_offset = 0;
        }

        /*!
         * Resets the buffer read cursor back to the beginning
         * of the packet.
         */
        inline void reset_read_cursor()
        {
            buffer_offset = 0;
        }

    private:
        /*!
         * Checks that there's enough data in the buffer to extract
         * a given number of bytes to prevent buffer overflows.
         * Throws an exception if there is not enough space.
         *
         * @param required_space The number of bytes needed
         */
        inline void assert_data_remaining(size_t required_space)
        {
            if(buffer_offset + required_space > buffer.size())
                throw std::out_of_range("Not enough bytes remaining in packet to extract requested");
        }

        std::string buffer; //Packet data buffer
        size_t buffer_offset; //Current read position
    };
}


#endif //FRNETLIB_PACKET_H
