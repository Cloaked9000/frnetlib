//
// Created by fred on 01/03/18.
//

#ifndef FRNETLIB_BASE64_H
#define FRNETLIB_BASE64_H
#include <string>

namespace fr
{
    class Base64
    {
    public:
        /*!
         * Encodes a string into Base64
         *
         * @param input The string to encode
         * @return The resulting encoded string
         */
        static std::string encode(const std::string &input);

        //There's no decode function at the moment. Maybe I'll write one eventually. Sorry.
    private:

    };
}

#endif //FRNETLIB_BASE64_H
