//
// Created by fred on 06/12/16.
//

#ifndef FRNETLIB_NETWORKENCODING_H
#define FRNETLIB_NETWORKENCODING_H

#include <netinet/in.h>
#include <cstring>

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


#endif //FRNETLIB_NETWORKENCODING_H
