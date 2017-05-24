//
// Created by fred on 10/12/16.
//

#include <iostream>
#include "frnetlib/HttpResponse.h"

namespace fr
{
    bool HttpResponse::parse(const std::string &response_data)
    {
        body += response_data;

        //Ensure that the whole header has been parsed first
        if(!header_ended)
        {
            //Check to see if this request data contains the end of the header
            auto header_end = body.find("\r\n\r\n");
            header_ended = header_end != std::string::npos;

            //If the header end has not been found, return true, indicating that we need more data.
            if(!header_ended)
            {
                return true;
            }
            else
            {
                parse_header(header_end);
                body.clear();
            }

            body += response_data.substr(header_end + 4, response_data.size() - header_end - 4);
        }

        return body.size() < content_length;

    }

    std::string HttpResponse::construct(const std::string &host) const
    {
        //Add HTTP header
        std::string response = "HTTP/1.1 " + std::to_string(status) + " \r\n";

        //Add the headers to the response
        for(const auto &header : header_data)
        {
            std::string data = header.first + ": " + header.second + "\r\n";
            response += data;
        }

        //Add in required headers if they're missing
        if(header_data.find("connection") == header_data.end())
            response += "connection: close_socket\r\n";
        if(header_data.find("content-type") == header_data.end())
            response += "content-type: text/html\r\n";

        //Add in space
        response += "\r\n";

        //Add in the body
        response += body + "\r\n";
        return response;
    }

    void HttpResponse::parse_header(ssize_t header_end_pos)
    {
        //Split the header into lines
        size_t line = 0;
        std::vector<std::string> header_lines = split_string(body.substr(0, header_end_pos));
        if(header_lines.empty())
            return;
        line++;

        //Read in headers
        for(; line < header_lines.size(); line++)
        {
            parse_header_line(header_lines[line]);
        }

        //Store content length value if it exists
        auto length_header_iter = header_data.find("content-length");
        if(length_header_iter != header_data.end())
            content_length = std::stoull(length_header_iter->second);
    }
}