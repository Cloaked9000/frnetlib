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
            uint16_t header_end_size = 4;
            auto header_end = body.find("\r\n\r\n");
            if(header_end == std::string::npos)
            {
                header_end = body.find("\n\n");
                header_end_size = 2;
            }
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

            body += response_data.substr(header_end + header_end_size, response_data.size() - header_end - header_end_size);
        }

        if(body.size() > content_length)
            body.resize(content_length);
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
        if(header_data.find("content-length") == header_data.end())
            response += "content-length: " + std::to_string(body.size()) + "\r\n";

        //Add in space
        response += "\r\n";

        //Add in the body
        response += body;
        return response;
    }

    bool HttpResponse::parse_header(int32_t header_end_pos)
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