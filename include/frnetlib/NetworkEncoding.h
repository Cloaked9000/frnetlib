//
// Created by fred on 06/12/16.
//

#ifndef FRNETLIB_NETWORKENCODING_H
#define FRNETLIB_NETWORKENCODING_H

#include <cstring>
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
#else
#define closesocket(x) close(x)
#define INVALID_SOCKET 0
#define SOCKET_ERROR (-1)

#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif


#undef htonll
#undef ntohll
#undef htonf
#undef ntohf
#undef htond
#undef ntohd
#define htonll(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))

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
    ret = htonll(ret);
    memcpy(&val, &ret, sizeof(val));
    return val;
}

inline double ntohd(double val)
{
    uint64_t ret;
    memcpy(&ret, &val, sizeof(ret));
    ret = ntohll(ret);
    memcpy(&val, &ret, sizeof(val));
    return val;
}

inline void set_unix_socket_blocking(int32_t socket_descriptor, bool is_blocking_already, bool should_block)
{
    //Don't update it if we're already in that mode
    if(should_block == is_blocking_already)
        return;

    //Different API calls needed for both windows and unix
#ifdef WIN32
    u_long non_blocking = should_block ? 0 : 1;
                        ioctlsocket(socket_descriptor, FIONBIO, &non_blocking);
#else
    int flags = fcntl(socket_descriptor, F_GETFL, 0);
    fcntl(socket_descriptor, F_SETFL, is_blocking_already ? flags ^ O_NONBLOCK : flags ^= O_NONBLOCK);
#endif
}

static void init_wsa()
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


#endif //FRNETLIB_NETWORKENCODING_H
