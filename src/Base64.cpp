//
// Created by fred on 01/03/18.
//

#include "frnetlib/Base64.h"

namespace fr
{
    std::string Base64::encode(const std::string &input)
    {
        static const std::string base64_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        std::string out;
        out.reserve(input.size() + (input.size() / 3) + 1);
        int a;

        //Do as many sets of 3 bytes as we can
        for(a = 0; a < input.size() - 2; a += 3)
        {
            out += base64_table[(input[a] >> 2) & 0x3F]; //Store the first 6 bits of the first byte
            out += base64_table[((input[a] & 0x3) << 4) | (input[a + 1] & 0xF0) >> 4]; //Store the last 2 bits of the first byte combined with the first 4 bits of the second byte
            out += base64_table[((input[a + 1] & 0xF) << 2) | (input[a + 2] & 0xC0) >> 6]; //Store the last 4 bits of the second byte combined with the first 2 bits of the third byte
            out += base64_table[input[a + 2] & 0x3F]; //Store the last 6 bits of the third byte
        }

        //Check if there's a remainder
        if(a < input.size())
        {
            out += base64_table[(input[a] >> 2) & 0x3F]; //Store first 6 bits of the first byte
            if(a == input.size() - 2) //There's 2 bytes left
            {
                out += base64_table[((input[a] & 0x3) << 4) | (input[a + 1] & 0xF0) >> 4]; //Store last 2 bits of first byte, and first 4 bits of second byte
                out += base64_table[(input[a + 1] & 0xF) << 2]; //Finally store the last 4 bits of the second byte
                out += "=";
            }
            else //There's 1 byte left
            {
                out += base64_table[(input[a] & 0x3) << 4]; //Store last 2 bits of first byte
                out += "==";
            }
        }

        return out;
    }

}
