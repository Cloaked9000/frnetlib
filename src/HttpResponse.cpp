//
// Created by fred on 10/12/16.
//

#include <iostream>
#include "frnetlib/HttpResponse.h"

namespace fr
{
    fr::Socket::Status HttpResponse::parse(const char *response_data, size_t datasz)
    {
        body += std::string(response_data, datasz);

        //Ensure that the whole header has been parsed first
        if(!header_ended)
        {
            //Verify that it's a valid HTTP response if there's enough data
            if(body.size() >= 4 && body.compare(0, 4, "HTTP") != 0)
                return fr::Socket::ParseError;

            //Check to see if this request data contains the end of the header
            uint16_t header_end_size = 4;
            auto header_end = body.find("\r\n\r\n");
            if(header_end == std::string::npos)
            {
                header_end = body.find("\n\n");
                header_end_size = 2;
            }
            header_ended = header_end != std::string::npos;

            //Ensure that the header doesn't exceed max length
            if((!header_ended && body.size() > MAX_HTTP_HEADER_SIZE) || (header_ended && header_end > MAX_HTTP_HEADER_SIZE))
            {
                return fr::Socket::HttpHeaderTooBig;
            }

            //If the header end has not been found, ask for more data.
            if(!header_ended)
                return fr::Socket::NotEnoughData;

            //Else parse it
            if(!parse_header(header_end))
                return fr::Socket::ParseError;

            //Leave things after the header intact
            body.erase(0, header_end + header_end_size);
        }

        //Ensure that body doesn't exceed maximum length
        if(body.size() > MAX_HTTP_BODY_SIZE)
            return fr::Socket::HttpBodyTooBig;

        //Cut off any data if it exceeds content length, todo: potentially an issue, could cut the next request off
        if(body.size() > content_length)
            body.resize(content_length);
        else if(body.size() < content_length)
            return fr::Socket::NotEnoughData;
        return fr::Socket::Success;

    }

    std::string HttpResponse::construct(const std::string &host) const
    {
        //Add HTTP header

        static_assert(RequestVersion::VersionCount == 3, "Update me");
        std::string response = ((version == RequestVersion::V1) ? "HTTP/1.0 " : "HTTP/1.1 ") + std::to_string(status) + " \r\n";

        //Add the headers to the response
        for(const auto &header : header_data)
        {
            std::string data = header.first + ": " + header.second + "\r\n";
            response += data;
        }

        //Add in required headers if they're missing
        if(header_data.find("connection") == header_data.end())
            response += "connection: keep-alive\r\n";
        if(header_data.find("content-type") == header_data.end())
            response += "content-type: text/html\r\n";
        if(header_data.find("content-length") == header_data.end())
            response += "content-length: " + std::to_string(body.size()) + "\r\n";

        //Add in space
        response += "\r\n";

        //Add in the body
        response += body;
        return response;
    }

    bool HttpResponse::parse_header(size_t header_end_pos)
    {
        try
        {
            //Split the header into lines
            size_t line = 0;
            std::vector<std::string> header_lines = split_string(body.substr(0, header_end_pos));
            if(header_lines.empty())
                return false;

            //Get response code
            auto status_begin = header_lines[0].find(' ');
            if(status_begin == std::string::npos)
                return false;
            auto end_pos = header_lines[0].find(' ', status_begin + 1);
            status = (RequestStatus)std::stoi(header_lines[0].substr(status_begin, end_pos - status_begin));

            //Get HTTP version
            static_assert(RequestVersion::VersionCount == 3, "Update me");
            version = header_lines[0].compare(0, status_begin, "HTTP/1.0") == 0 ? RequestVersion::V1 : RequestVersion::V1_1;
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
        catch(const std::exception &e)
        {
            return false;
        }
        return true;
    }
}