//
// Created by fred on 06/12/16.
//

#ifndef FRNETLIB_NETWORKENCODING_H
#define FRNETLIB_NETWORKENCODING_H

#include <cstring>
#include <string>
#include <cstdint>
#include <atomic>
#include <exception>
#include <stdexcept>
#include <csignal>

//Windows and UNIX require some different headers.
//We also need some compatibility defines for cross platform support.
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#define SOL_TCP SOL_SOCKET
#define SHUT_RDWR SD_BOTH
#else

#ifdef __GNUC__
#define UNUSED_VAR __attribute__ ((unused))
#else
# define UNUSED_VAR
#endif

#define closesocket(x) close(x)
#define INVALID_SOCKET 0
#define SOCKET_ERROR (-1)

#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#endif


#if defined(__GNUC__)
#  if __BYTE_ORDER == __LITTLE_ENDIAN
#   define fr_htonll(x) __builtin_bswap64 (x)
#   define fr_ntohll(x) __builtin_bswap64 (x)
#  else
#   define fr_htonll(x) (x)
#   define fr_ntohll(x) (x)
#  endif
#else
# define fr_htonll(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
# define fr_ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))
#endif

#undef htonf
#undef htond
#undef ntohd
#undef ntonf

namespace fr
{
    inline float htonf(float val)
    {
        uint32_t ret;
        memcpy(&ret, &val, sizeof(ret));
        ret = htonl(ret);
        memcpy(&val, &ret, sizeof(val));
        return val;
    }

inline float ntohf(float val)
{
    uint32_t ret;
    memcpy(&ret, &val, sizeof(ret));
    ret = ntohl(ret);
    memcpy(&val, &ret, sizeof(val));
    return val;
}

inline double htond(double val)
{
    uint64_t ret;
    memcpy(&ret, &val, sizeof(ret));
    ret = fr_htonll(ret);
    memcpy(&val, &ret, sizeof(val));
    return val;
}

inline double ntohd(double val)
{
    uint64_t ret;
    memcpy(&ret, &val, sizeof(ret));
    ret = fr_ntohll(ret);
    memcpy(&val, &ret, sizeof(val));
    return val;
}

inline bool set_unix_socket_blocking(int32_t socket_descriptor, bool is_blocking_already, bool should_block)
{
    //Don't update it if we're already in that mode
    if(should_block == is_blocking_already)
        return true;

    //Different API calls needed for both windows and unix
#ifdef WIN32
    u_long non_blocking = should_block ? 0 : 1;
    int ret = ioctlsocket(socket_descriptor, FIONBIO, &non_blocking);
    if(ret != 0)
        return false;
#else
    int flags = fcntl(socket_descriptor, F_GETFL, 0);
    if(flags < 0)
        return false;
    flags = fcntl(socket_descriptor, F_SETFL, is_blocking_already ? flags ^ O_NONBLOCK : flags ^= O_NONBLOCK);
    if(flags < 0)
        return false;
#endif
    return true;
}

inline static void init_wsa()
{
#ifdef _WIN32
    static WSADATA wsaData = WSAData();
    static std::atomic<uint32_t> instance_count{0};
    if(instance_count++ == 0)
    {
        int wsa_result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if(wsa_result != 0)
        {
            throw std::runtime_error("Failed to initialise WSA: " + std::to_string(wsa_result));
        }
    }
#else
    signal(SIGPIPE, SIG_IGN);
#endif
}
}


#endif //FRNETLIB_NETWORKENCODING_H
