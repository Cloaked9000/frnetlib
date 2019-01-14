//
// Created by fred on 06/12/16.
//

#include <mutex>
#include <csignal>
#include <iostream>
#include <vector>
#ifdef USE_SSL
#include <mbedtls/error.h>
#endif
#include "frnetlib/NetworkEncoding.h"
#include "frnetlib/Socket.h"
#include "frnetlib/Sendable.h"

namespace fr
{
    Socket::Socket()
    : is_blocking(true),
      ai_family(AF_UNSPEC),
      max_receive_size(0),
      socket_read_timeout(0)
    {
        init_wsa();
    }

    Socket::Status Socket::send(const Sendable &obj)
    {
        if(!connected())
            return Socket::Disconnected;

        return obj.send(this);
    }

    Socket::Status Socket::receive(Sendable &obj)
    {
        if(!connected())
            return Socket::Disconnected;

        return obj.receive(this);
    }

    Socket::Status Socket::receive_all(void *dest, size_t buffer_size)
    {
        if(!connected())
            return Socket::Disconnected;

        auto bytes_remaining = (int32_t) buffer_size;
        size_t bytes_read = 0;
        while(bytes_remaining > 0)
        {
            size_t received = 0;
            auto *arr = (char*)dest;
            Status status = receive_raw(&arr[bytes_read], (size_t)bytes_remaining, received);
            if(status != fr::Socket::WouldBlock && status != fr::Socket::Success)
                return status;
            bytes_remaining -= received;
            bytes_read += received;
            if(status == fr::Socket::WouldBlock && bytes_read == 0)
                return status;
        }

        return Socket::Status::Success;
    }

    void Socket::shutdown()
    {
        ::shutdown(get_socket_descriptor(), SHUT_RDWR);
    }

    void Socket::set_inet_version(Socket::IP version)
    {
        switch(version)
        {
            case Socket::IP::v4:
                ai_family = AF_INET;
                break;
            case Socket::IP::v6:
                ai_family = AF_INET6;
                break;
            case Socket::IP::any:
                ai_family = AF_UNSPEC;
                break;
            default:
                throw std::logic_error("Unknown Socket::IP value passed to set_inet_version()");
        }
    }

    std::string Socket::status_to_string(fr::Socket::Status status)
    {
#ifdef _WIN32
        auto wsa_err_to_str = [](int err) -> std::string {
        std::string buff(255, '\0');
        auto len = FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT), &buff[0], buff.size(), NULL);
        if(len == 0)
            return "Unknown";
        buff.resize(len);
        return buff;
        };
        #define ERR_STR wsa_err_to_str(WSAGetLastError())
#else
    #define ERR_STR strerror(errno)
#endif

        switch(status)
        {
            case Unknown:
                return "Unknown";
            case Success:
                return "Success";
            case ListenFailed:
                return std::string("Listen Failed (").append(ERR_STR).append(")");
            case BindFailed:
                return std::string("Bind Failed (").append(ERR_STR).append(")");
            case Disconnected:
                return "The Socket Is Not Connected";
            case Error:
                return "Error";
            case WouldBlock:
                return "Would Block";
            case ConnectionFailed:
                return "Connection Failed";
            case HandshakeFailed:
                return "Handshake Failed";
            case VerificationFailed:
                return "Verification Failed";
            case MaxPacketSizeExceeded:
                return "Max Packet Size Exceeded";
            case NotEnoughData:
                return "Not Enough Data";
            case ParseError:
                return "Parse Error";
            case HttpHeaderTooBig:
                return "HTTP Header Too Big";
            case HttpBodyTooBig:
                return "HTTP Body Too Big";
            case AddressLookupFailure:
#ifdef _WIN32
                return std::string("Address Lookup Failure (").append(wsa_err_to_str(WSAGetLastError())).append(")");
#else
                return std::string("Address Lookup Failure (").append(gai_strerror(errno)).append(")");
#endif
            case SendError:
                return std::string("Send Error (").append(ERR_STR).append(")");
            case ReceiveError:
                return std::string("Receive Error (").append(ERR_STR).append(")");
            case AcceptError:
                return std::string("Accept Error (").append(ERR_STR).append(")");
            case SSLError:
            {
#ifdef USE_SSL
                char buff[256] = {0};
                mbedtls_strerror(errno, buff, sizeof(buff));
                return std::string("SSL Error (").append(buff).append(")");
#else
                return "Generic SSL Error";
#endif
            }
            default:
                return "Unknown";
        }

        return "Internal Error";
    }

    void Socket::disconnect()
    {
        close_socket();
    }
}