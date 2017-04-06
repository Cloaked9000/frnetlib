//
// Created by fred on 10/12/16.
//

#ifndef FRNETLIB_HTTPREQUEST_H
#define FRNETLIB_HTTPREQUEST_H
#include <string>
#include <vector>
#include <unordered_map>
#include "TcpSocket.h"
#include "Http.h"

namespace fr
{
    class HttpRequest : public Http
    {
    public:
        //Constructors
        HttpRequest() = default;
        HttpRequest(HttpRequest &&other) = default;

        /*!
         * Parse a HTTP response.
         *
         * @param data The HTTP response to parse
         * @return True if more data is needed, false if finished.
         */
        bool parse(const std::string &data) override;

        /*!
         * Constructs a Http Request, ready to send.
         *
         * @return The constructed HTTP request.
         */
        std::string construct(const std::string &host) const override;
    };
}


#endif //FRNETLIB_HTTPREQUEST_H
