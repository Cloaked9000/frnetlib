//
// Created by fred on 01/03/18.
//

#ifndef FRNETLIB_SHA1_H
#define FRNETLIB_SHA1_H

#include <string>
#include "frnetlib/NetworkEncoding.h"

namespace fr
{
    class Sha1
    {
    public:
        Sha1();

        /*!
         * Sha1 hashes a string input and returns the raw digest
         *
         * @param input The string to hash
         * @return The Sha1 digest converted to host endianness
         */
        static std::string sha1_digest(const std::string &input)
        {
            Sha1 ctx;
            ctx.update(input);
            ctx.final();
            for(unsigned int &a : ctx.digest)
                a = ntohl(a);

            return std::string((char*)&ctx.digest[0], sizeof(ctx.digest));
        }

    private:
        void update(const std::string &s);
        void update(std::istream &is);
        void final();

        uint32_t digest[5];
        std::string buffer;
        uint64_t transforms;
    };
}

#endif //FRNETLIB_SHA1_H
