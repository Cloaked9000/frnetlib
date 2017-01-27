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
#define PACKET_HEADER_LENGTH sizeof(uint32_t)
    class Packet
    {
    public:
        Packet() noexcept
        : buffer_read_index(PACKET_HEADER_LENGTH),
          buffer(PACKET_HEADER_LENGTH, '0')
        {

        }

        //Nasty constructor to allow things like Packet{1, 2, 3, "bob"}.
        template <typename T, typename ...Args>
        Packet(T const &part, Args &&...args)
        : Packet()
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
         * For packing pairs
         */
        template<typename A, typename B>
        inline Packet &operator<<(const std::pair<A, B> &var)
        {
            *this << var.first;
            *this << var.second;
            return *this;
        }

        /*
         * For extracting pairs
         */
        template<typename A, typename B>
        inline Packet &operator>>(std::pair<A, B> &var)
        {
            *this >> var.first;
            *this >> var.second;
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

            memcpy(&var, &buffer[buffer_read_index], sizeof(var));
            buffer_read_index += sizeof(var);
            return *this;
        }

        /*
         * Adds an 8bit variable to the packet
         */
        inline Packet &operator<<(uint8_t var)
        {
            buffer.resize(buffer.size() + sizeof(var));
            memcpy(&buffer[buffer.size() - sizeof(var)], &var, sizeof(var));
            return *this;
        }

        /*
         * Extracts an 8bit variable from the packet
         */
        inline Packet &operator>>(uint8_t &var)
        {
            assert_data_remaining(sizeof(var));

            memcpy(&var, &buffer[buffer_read_index], sizeof(var));
            buffer_read_index += sizeof(var);
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

            memcpy(&var, &buffer[buffer_read_index], sizeof(var));
            buffer_read_index += sizeof(var);
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

            memcpy(&var, &buffer[buffer_read_index], sizeof(var));
            buffer_read_index += sizeof(var);
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

            memcpy(&var, &buffer[buffer_read_index], sizeof(var));
            buffer_read_index += sizeof(var);
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

            memcpy(&var, &buffer[buffer_read_index], sizeof(var));
            buffer_read_index += sizeof(var);
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

            memcpy(&var, &buffer[buffer_read_index], sizeof(var));
            buffer_read_index += sizeof(var);
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

            memcpy(&var, &buffer[buffer_read_index], sizeof(var));
            buffer_read_index += sizeof(var);
            var = ntohll((uint64_t)var);
            return *this;
        }

        /*
         * Adds an enum, or enum class to the packet.
         *
         * Underlying type must not be larger than 32bits.
         * The enum's underlying type should be specified like so:
         *
         * enum Enum : type{};
         */
        template<typename T, typename std::enable_if<std::is_enum<T>::value>::type* = nullptr>
        inline Packet &operator<<(T var)
        {
            *this << (uint32_t)static_cast<typename std::underlying_type<T>::type>(var);
            return *this;
        }

        /*
         * Extracts en enum, or enum class from the packet.
         *
         * Underlying type must not be larger than 32bits.
         * The enum's underlying type should be specified like so:
         *
         * enum Enum : type{};
         */
        template<typename T, typename std::enable_if<std::is_enum<T>::value>::type* = nullptr>
        inline Packet &operator>>(T &var)
        {
            uint32_t val;
            *this >> val;
            var = static_cast<T>(val);
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

            memcpy(&var, &buffer[buffer_read_index], sizeof(var));
            buffer_read_index += sizeof(var);
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

            memcpy(&var, &buffer[buffer_read_index], sizeof(var));
            buffer_read_index += sizeof(var);;
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

            var = buffer.substr(buffer_read_index, length);
            buffer_read_index += length;

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
            buffer.erase(PACKET_HEADER_LENGTH, buffer.size() - PACKET_HEADER_LENGTH);
            buffer_read_index = PACKET_HEADER_LENGTH;
        }

        /*!
         * Resets the read cursor back to 0, or a specified position.
         *
         * @param pos The buffer index to continue reading from.
         */
        inline void reset_read_cursor(size_t pos = 0)
        {
            buffer_read_index = PACKET_HEADER_LENGTH + pos;
        }

        /*!
         * Reserves space in the internal packet buffer,
         * for if you know how much data you expect to receive
         * or send.
         *
         * @param bytes The number of bytes to reserve
         */
        inline void reserve(size_t bytes)
        {
            buffer.reserve(PACKET_HEADER_LENGTH + bytes);
        }

    private:
        friend class Socket;

        /*!
         * Gets the data added to the packet
         *
         * @return A string containing all of the data added to the packet
         */
        inline std::string &get_buffer()
        {
            //Update packet length first
            uint32_t length = htonl((uint32_t)buffer.size() - PACKET_HEADER_LENGTH);
            memcpy(&buffer[0], &length, sizeof(uint32_t));

            //Then a reference to the buffer
            return buffer;
        }

        /*!
         * Checks that there's enough data in the buffer to extract
         * a given number of bytes to prevent buffer overflows.
         * Throws an exception if there is not enough space.
         *
         * @param required_space The number of bytes needed
         */
        inline void assert_data_remaining(size_t required_space)
        {
            if(buffer_read_index + required_space > buffer.size())
                throw std::out_of_range("Not enough bytes remaining in packet to extract requested");
        }


        std::string buffer; //Packet data buffer
        size_t buffer_read_index; //Current read position
    };
}


#endif //FRNETLIB_PACKET_H
