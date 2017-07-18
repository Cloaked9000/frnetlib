//
// Created by fred on 10/12/16.
//

#ifndef FRNETLIB_HTTPRESPONSE_H
#define FRNETLIB_HTTPRESPONSE_H

#include <string>
#include <vector>
#include <unordered_map>
#include "Http.h"

namespace fr
{
    class HttpResponse : public Http
    {
    public:
        //Constructors
        HttpResponse(){};
        virtual ~HttpResponse(){}

        /*!
         * Parse a HTTP response.
         *
         * @param data The HTTP response to parse
         * @param datasz The length of data in bytes
         * @return True if more data is needed, false if finished.
         */
        bool parse(const char *data, size_t datasz) override;

        /*!
         * Constructs a HttpResponse, ready to send.
         *
         * @return The constructed HTTP response.
         */
        std::string construct(const std::string &host) const override;

    private:
        /*!
         * Parses the request header.
         *
         * @param header_end_pos The position in 'body' of the end of the header
         */
        bool parse_header(int32_t header_end_pos);

        //State
        bool header_ended;
        size_t content_length;
    };
}

#endif //FRNETLIB_HTTPRESPONSE_H
