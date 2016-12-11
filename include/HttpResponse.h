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
        HttpResponse() = default;
        HttpResponse(HttpResponse &&other) = default;

        /*!
         * Parse a HTTP response.
         *
         * @param data The HTTP response to parse
         */
        void parse(const std::string &data) override;

        /*!
         * Constructs a HttpResponse, ready to send.
         *
         * @return The constructed HTTP response.
         */
        std::string construct() const override;
    };
}

#endif //FRNETLIB_HTTPRESPONSE_H
