//
// Created by fred on 06/12/16.
//

#ifndef FRNETLIB_PACKET_H
#define FRNETLIB_PACKET_H
#include <string>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <map>
#include <unordered_map>
#include "NetworkEncoding.h"
#include "Packetable.h"
#include "Sendable.h"

#ifdef __cpp_lib_string_view
#include <string_view>
#endif

namespace fr
{
#define PACKET_HEADER_LENGTH sizeof(uint32_t)
    class Packet : public Sendable
    {
    public:
        Packet() noexcept
        : buffer(PACKET_HEADER_LENGTH, '0'),
          buffer_read_index(PACKET_HEADER_LENGTH)
        {

        }

        //Nasty constructor to allow things like Packet{1, 2, 3, "bob"}.
        template <typename T, typename ...Args>
        Packet(T const &part, Args &&...args)
        : Packet()
        {
            add(part, std::forward<Args>(args)...);
        }

        /*!
         * Variadic add function for adding multiple values at once to the packet,
         * used by packet constructor.
         *
         * @param part The first argument to add
         * @param args The remaining arguments to add
         */
        template<typename T, typename ...Args>
        inline void add(T const& part, Args &&...args)
        {
            *this << part;
            add(std::forward<Args>(args)...);
        }

        /*!
         * Part of the variadic add function.
         *
         * @param part The argument to add to the packet
         */
        template<typename T>
        inline void add(T const &part)
        {
            *this << part;
        }

        /*!
         * Add function to allow adding iterator ranges to the packet.
         *
         * @note std::distance() is used, and so this function should ideally be used with random access
         * iterators to achieve constant time complexity.
         * @tparam Iter The iterator type
         * @param begin An iterator to the first element to add from
         * @param end A past-the-end iterator to stop adding at
         */
        template<typename Iter>
        inline void add_range(Iter begin, Iter end)
        {
            *this << static_cast<uint32_t>(std::distance(begin, end));
            for(auto iter = begin; iter != end; ++iter)
                *this << *iter;
        }

        /*!
         * Adds raw data to packet
         *
         * @param data The data to add
         * @param datasz The length of data
         */
        inline void add_raw(const char *data, size_t datasz)
        {
            buffer.append(data, datasz);
        }

        /*!
         * Extracts raw data from a packet
         *
         * @param data Where to store the extracted data
         * @param datasz The number of bytes to extract
         */
        inline void extract_raw(char *data, size_t datasz)
        {
            assert_bytes_remaining(datasz);

            memcpy(data, &buffer[buffer_read_index], datasz);
            buffer_read_index += datasz;
        }

        /*
         * Adds a vector to a packet
         */
        template<typename T>
        inline Packet &operator<<(const std::vector<T> &vec)
        {
            //First store its length
            *this << static_cast<uint32_t>(vec.size());

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
            uint32_t length;

            //First extract the length
            *this >> length;
            vec.resize(length);

            //Now take each of the elements out of the packet
            for(auto &&iter : vec)
            {
                *this >> iter;
            }
            return *this;
        }

        /*
         * Adds a map to a packet
         */
        template<typename A, typename B>
        inline Packet &operator<<(const std::map<A, B> &m)
        {
            //First store its length
            *this << static_cast<uint32_t>(m.size());

            //Now each of the elements
            for(const auto &iter : m)
            {
                *this << iter;
            }

            return *this;
        }

        /*
         * Extracts a map from the packet
         */
        template<typename A, typename B>
        inline Packet &operator>>(std::map<A, B> &m)
        {
            uint32_t length;

            //First extract the length
            *this >> length;

            //Now take each of the elements out of the packet
            for(uint32_t a = 0; a < length; a++)
            {
                std::pair<A, B> pair;
                *this >> pair;
                m.emplace(std::move(pair));
            }

            return *this;
        }

        /*
         * Adds an unordered_map to a packet
         */
        template<typename A, typename B>
        inline Packet &operator<<(const std::unordered_map<A, B> &m)
        {
            //First store its length
            *this << static_cast<uint32_t>(m.size());

            //Now each of the elements
            for(const auto &iter : m)
            {
                *this << iter;
            }

            return *this;
        }

        /*
         * Extracts an unordered_map from the packet
         */
        template<typename A, typename B>
        inline Packet &operator>>(std::unordered_map<A, B> &m)
        {
            uint32_t length;

            //First extract the length
            *this >> length;

            //Now take each of the elements out of the packet
            for(uint32_t a = 0; a < length; a++)
            {
                std::pair<A, B> pair;
                *this >> pair;
                m.emplace(std::move(pair));
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
            buffer.append((char*)&var, sizeof(var));
            return *this;
        }

        /*
         * Extracts a boolean variable from the packet
         */
        inline Packet &operator>>(bool &var)
        {
            assert_bytes_remaining(sizeof(var));

            memcpy(&var, &buffer[buffer_read_index], sizeof(var));
            buffer_read_index += sizeof(var);
            return *this;
        }

        /*
         * Adds an 8bit variable to the packet
         */
        inline Packet &operator<<(uint8_t var)
        {
            buffer.append((char*)&var, sizeof(var));
            return *this;
        }

        /*
         * Extracts an 8bit variable from the packet
         */
        inline Packet &operator>>(uint8_t &var)
        {
            assert_bytes_remaining(sizeof(var));

            memcpy(&var, &buffer[buffer_read_index], sizeof(var));
            buffer_read_index += sizeof(var);
            return *this;
        }

        /*
         * Adds a 16bit variable to the packet
         */
        inline Packet &operator<<(uint16_t var)
        {
            var = htons(var);
            buffer.append((char*)&var, sizeof(var));
            return *this;
        }

        /*
         * Extracts a 16bit variable from the packet
         */
        inline Packet &operator>>(uint16_t &var)
        {
            assert_bytes_remaining(sizeof(var));

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
            buffer.append((char*)&var, sizeof(var));
            return *this;
        }

        /*
         * Extracts a 32bit variable from the packet
         */
        inline Packet &operator>>(uint32_t &var)
        {
            assert_bytes_remaining(sizeof(var));
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
            var = fr_htonll(var);
            buffer.append((char*)&var, sizeof(var));
            return *this;
        }

        /*
         * Extracts a 64bit variable from the packet
         */
        inline Packet &operator>>(uint64_t &var)
        {
            assert_bytes_remaining(sizeof(var));

            memcpy(&var, &buffer[buffer_read_index], sizeof(var));
            buffer_read_index += sizeof(var);
            var = fr_ntohll(var);
            return *this;
        }

        /*
         * Adds a 16bit variable to the packet
         */
        inline Packet &operator<<(int16_t var)
        {
            var = htons((uint16_t)var);
            buffer.append((char*)&var, sizeof(var));
            return *this;
        }

        /*
         * Extracts a 16bit variable from the packet
         */
        inline Packet &operator>>(int16_t &var)
        {
            assert_bytes_remaining(sizeof(var));

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
            var = htonl((uint32_t)var);
            buffer.append((char*)&var, sizeof(var));
            return *this;
        }

        /*
         * Extracts a 32bit variable from the packet
         */
        inline Packet &operator>>(int32_t &var)
        {
            assert_bytes_remaining(sizeof(var));

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
            var = fr_htonll((uint64_t)var);
            buffer.append((char*)&var, sizeof(var));
            return *this;
        }

        /*
         * Extracts a 64bit variable from the packet
         */
        inline Packet &operator>>(int64_t &var)
        {
            assert_bytes_remaining(sizeof(var));

            memcpy(&var, &buffer[buffer_read_index], sizeof(var));
            buffer_read_index += sizeof(var);
            var = fr_ntohll((uint64_t)var);
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
            static_assert(sizeof(typename std::underlying_type<T>::type) <= 4, "Enum types must not be larger than 32bits");
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
            static_assert(sizeof(typename std::underlying_type<T>::type) <= 4, "Enum types must not be larger than 32bits");
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
            var = htonf(var);
            buffer.append((char*)&var, sizeof(var));
            return *this;
        }

        /*
         * Extracts a float variable from the packet
         */
        inline Packet &operator>>(float &var)
        {
            assert_bytes_remaining(sizeof(var));

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
            var = htond(var);
            buffer.append((char*)&var, sizeof(var));
            return *this;
        }

        /*
         * Extracts a double variable from the packet
         */
        inline Packet &operator>>(double &var)
        {
            assert_bytes_remaining(sizeof(var));

            memcpy(&var, &buffer[buffer_read_index], sizeof(var));
            buffer_read_index += sizeof(var);;
            var = ntohd(var);
            return *this;
        }

#ifdef __cpp_lib_string_view
        /*!
         * Pack an std::string_view. This will make a copy.
         * May be extracted as either an std::string or an std::string_view
         */
        inline Packet &operator<<(std::string_view view)
        {
            //Same as std::string
            *this << (uint32_t)view.size();
            buffer.append(view.data(), view.size());
            return *this;
        }

        /*!
         * Extracts a string as an std::string_view.
         *
         * @note Be very careful with this. If the packet is destroyed then the string_view will be too!
         */
        inline Packet&operator>>(std::string_view &var)
        {
            uint32_t length;
            *this >> length;

            assert_bytes_remaining(length);
            var = std::string_view(&buffer[buffer_read_index], length);
            buffer_read_index += length;
            return *this;
        }
#endif

        /*
         * Adds a string variable to the packet
         */
        inline Packet &operator<<(const std::string &var)
        {
            //Strings are prefixed with their length as a 32bit uint :)
            *this << (uint32_t)var.length();
            buffer.append(var);
            return *this;
        }

        /*
         * Removes a string variable from the packet
         */
        inline Packet&operator>>(std::string &var)
        {
            uint32_t length;
            *this >> length;

            assert_bytes_remaining(length);
            var.assign(&buffer[buffer_read_index], length);
            buffer_read_index += length;

            return *this;
        }

        /*
         * Removes a boolean from the packet into a reference
         */
        inline Packet&operator>>(std::vector<bool>::reference &ref)
        {
            bool b;
            *this >> b;
            ref = b;
            return *this;
        }


        /*
         * Adds a char array
         */
        inline Packet &operator<<(const char *var)
        {
            auto len = (uint32_t)strlen(var);
            *this << len;
            buffer.append(var, len);
            return *this;
        }

        /*!
         * Should be called to pack an object that inherits
         * fr::Packetable to the packet.
         *
         * @param object The class to pack
         * @return The current packet object for chaining
         */
        inline Packet &operator<<(const fr::Packetable &object)
        {
            object.pack(*this);
            return *this;
        }


        /*!
         * Should be called to unpack an object that inherits
         * fr::Packetable.
         *
         * @param object The object to unpack into
         * @return The current packet object for chaining
         */
        inline Packet &operator>>(fr::Packetable &object)
        {
            object.unpack(*this);
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
            //Leave enough for the header
            buffer.clear();
            buffer.append(PACKET_HEADER_LENGTH, '\0');
            buffer_read_index = PACKET_HEADER_LENGTH;
        }

        /*!
         * Sets the read cursor back to 0, or a specific position.
         *
         * @note THIS IS AN *ABSOLUTE* SEEK
         * @param pos The buffer index to continue reading from.
         */
        inline void set_cursor(size_t pos = 0)
        {
            buffer_read_index = PACKET_HEADER_LENGTH + pos;
            if(buffer_read_index > buffer.size()) buffer_read_index = buffer.size();
        }

        /*!
         * Gets the current read cursor position within the packet
         *
         * @return Current read position
         */
        inline size_t get_cursor() const
        {
            return buffer_read_index - PACKET_HEADER_LENGTH;
        }

        /*!
         * Relative seek of the read cursor, to seek to a specific position within the packet.
         * If the seek is out of bounds, then the offset will be set to the nearest valid value.
         * So seeking to -10, would seek to 0. Seeking to 110/100 would seek to 100.
         *
         * @param pos The number of bytes to seek in either direction
         */
        inline void seek_cursor(ssize_t pos)
        {
            ssize_t new_offset = buffer_read_index + pos;
            if(new_offset < (ssize_t)PACKET_HEADER_LENGTH)
                new_offset = PACKET_HEADER_LENGTH;
            if(new_offset > (ssize_t)buffer.size())
                new_offset = buffer.size();
            buffer_read_index = static_cast<size_t>(new_offset);
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

        /*!
         * Checks that there's enough data in the buffer to extract
         * a given number of bytes to prevent buffer overflows.
         *
         * @throws an std::out_of_range exception if there is not enough space.
         * @param required_space The number of bytes needed
         */
        inline void assert_bytes_remaining(size_t required_space) const
        {
            if(buffer_read_index + required_space > buffer.size())
            {
                throw std::out_of_range("Not enough bytes remaining in packet to extract requested data");
            }
        }

        /*!
         * Gets the number of bytes available until the end of the packet
         *
         * @return The number of bytes available until the end of the packet
         */
        inline size_t get_bytes_remaining() const
        {
            return buffer.size() - buffer_read_index;
        }

        /*!
         * Gets the size of the packet in bytes, this does not take into
         * account the cursor position, use get_bytes_remaining() if you want to
         * see how many more bytes can be extracted.
         *
         * @note This does not include the packet header length
         *
         * @return The size in bytes of the packet
         */
        inline size_t size() const
        {
            return buffer.size() <= PACKET_HEADER_LENGTH ? 0 : buffer.size() - PACKET_HEADER_LENGTH;
        }

        /*!
         * Overridable send, to allow
         * custom types to be directly sent through
         * sockets.
         *
         * @param socket The socket to send through
         * @return Status indicating if the send succeeded or not.
         */
        Socket::Status send(Socket *socket) const override
        {
            uint32_t length = htonl((uint32_t)buffer.size() - PACKET_HEADER_LENGTH);
            memcpy(&buffer[0], &length, sizeof(uint32_t));
            fr::Socket::Status state;
            size_t sent = 0;
            do
            {
                state = socket->send_raw(&buffer[0], buffer.size(), sent);
            } while(state == fr::Socket::Status::WouldBlock);
            return state;
        }

        /*!
         * Overrideable receive, to allow
         * custom types to be directly received through
         * sockets.
         *
         * @return Status indicating if the send succeeded or not:
         * 'Success': All good, object still valid.
         * 'WouldBlock' or 'Timeout': No data received. Object still valid though.
         * Anything else: Object invalid. Call disconnect().
         */
        Socket::Status receive(Socket *socket) override
        {
            Socket::Status status;

            //Try to read packet length
            uint32_t packet_length = 0;
            status = socket->receive_all(&packet_length, sizeof(packet_length));
            if(status != Socket::Status::Success)
                return status;
            packet_length = ntohl(packet_length);

            //Check that packet_length doesn't exceed the limit, if any
            if(socket->get_max_receive_size() && packet_length > socket->get_max_receive_size())
                return fr::Socket::Status::MaxPacketSizeExceeded;

            //Now we've got the length, read the rest of the data in
            if(packet_length + PACKET_HEADER_LENGTH > buffer.size())
                buffer.resize(packet_length + PACKET_HEADER_LENGTH);

            do
            {
                status = socket->receive_all(&buffer[PACKET_HEADER_LENGTH], packet_length);
            } while(status == fr::Socket::Status::WouldBlock);
            if(status == fr::Socket::Status::Timeout)
                status = fr::Socket::Status::Disconnected;
            return status;
        }

    private:
        friend class Socket;


        mutable std::string buffer; //Packet data buffer
        size_t buffer_read_index; //Current read position
    };
}


#endif //FRNETLIB_PACKET_H
