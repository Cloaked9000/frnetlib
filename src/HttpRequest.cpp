//
// Created by fred on 10/12/16.
//

#include <algorithm>
#include <iostream>
#include "frnetlib/HttpRequest.h"
namespace fr
{
     HttpRequest::HttpRequest()
    : header_ended(false),
      last_parsed_character(0),
      content_length(0)
    {

    }

    fr::Socket::Status HttpRequest::parse(const char *request, size_t requestsz)
    {
        body += std::string(request, requestsz);

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

            //Ensure that the header doesn't exceed max length
            if(!header_ended && body.size() > MAX_HTTP_HEADER_SIZE || header_ended && header_end > MAX_HTTP_HEADER_SIZE)
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
        {
            return fr::Socket::HttpBodyTooBig;
        }

        //If we've got the whole request, parse the POST if it exists
        if(body.size() >= content_length)
        {
            if(request_type == RequestType::Post)
                parse_post_body();
            return fr::Socket::Success;
        }

        return fr::Socket::NotEnoughData;
    }

    bool HttpRequest::parse_header(int64_t header_end_pos)
    {
        try
        {
            //Split the header into lines
            size_t line = 0;
            std::vector<std::string> header_lines = split_string(body.substr(0, (unsigned long)header_end_pos));
            if(header_lines.empty())
                return true;

            //Parse request type & uri
            parse_header_type(header_lines[line]);
            parse_header_uri(header_lines[line]);
            line++;

            //Read in headers
            for(; line < header_lines.size(); line++)
                parse_header_line(header_lines[line]);

            //Store content length value if it exists
            auto length_header_iter = header_data.find("content-length");
            if(length_header_iter != header_data.end())
                content_length = (size_t)std::stoull(length_header_iter->second);
        }
        catch(const std::exception &e)
        {
            return false;
        }
        return true;

    }

    std::string HttpRequest::construct(const std::string &host) const
    {
        //Add HTTP header
        std::string request = request_type_to_string(request_type == Http::Unknown ? Http::Get : request_type) + " " + uri;
        if(!get_data.empty())
        {
            request += "?";
            for(auto iter = get_data.begin(); iter != get_data.end();)
            {
                request += iter->first + "=" + iter->second;
                if(++iter != get_data.end())
                    request += "&";
            }
        }
        request += " HTTP/1.1\r\n";

        //Add the headers to the request
        for(const auto &header : header_data)
        {
            std::string data = header.first + ": " + header.second + "\r\n";
            request += data;
        }

        //Generate post line
        std::string post_string;
        for(auto iter = post_data.begin(); iter != post_data.end();)
        {
            post_string += iter->first + "=" + iter->second;
            if(++iter != get_data.end())
                post_string += "&";
        }
        post_string += "\r\n";

        //Add in required headers if they're missing
        if(header_data.find("Connection") == header_data.end())
            request += "Connection: keep-alive\n";
        if(header_data.find("Host") == header_data.end())
            request += "Host: " + host + "\r\n";
        if(!body.empty())
            request += "Content-Length: " + std::to_string(body.size() + post_string.size()) + "\r\n";

        //Add in space
        request += "\r\n";

        //Add in post
        request += post_string;

        //Add in the body
        request += body;

        return request;
    }

    void HttpRequest::parse_post_body()
    {
        //Find beginning of post data
        auto post_begin = body.find_first_not_of("\r\n");
        if(post_begin == std::string::npos)
            post_begin = body.find_first_not_of('\n');

        //Find end of post data
        auto post_end = body.rfind("\r\n\r\n");
        if(post_end == std::string::npos)
            post_end = body.rfind("\n\n");

        //Sanity check
        if(post_begin == post_end || post_begin == std::string::npos)
            return;

        //Split up the body and store each argument name and value
        auto post = parse_argument_list(body.substr(post_begin, body.size() - post_begin - (body.size() - post_end)));
        for(auto &c : post)
        {
            std::transform(c.first.begin(), c.first.end(), c.first.begin(), ::tolower);
            post_data.emplace(std::move(c.first), std::move(c.second));
        }
    }

    void HttpRequest::parse_header_type(const std::string &str)
    {
        //Find the request type
        auto type_end = str.find(' ');
        if(type_end != std::string::npos)
        {
            //Check what it is
            if(str.compare(0, type_end, "GET") == 0)
                request_type = fr::Http::Get;
            else if(str.compare(0, type_end, "POST") == 0)
                request_type = fr::Http::Post;
            else if(str.compare(0, type_end, "PUT") == 0)
                request_type = fr::Http::Put;
            else if(str.compare(0, type_end, "DELETE") == 0)
                request_type = fr::Http::Delete;
            else if(str.compare(0, type_end, "PATCH") == 0)
                request_type = fr::Http::Patch;
            else
                request_type = fr::Http::Unknown;

            return;
        }
        throw std::invalid_argument("No known request type found in: " + str);
    }

    void HttpRequest::parse_header_uri(const std::string &str)
    {
        auto uri_begin = str.find('/');
        auto uri_end = str.find("HTTP") - 1;
        if(uri_begin != std::string::npos)
        {
            //Parse GET variables
            auto get_begin = str.find('?');
            if(get_begin != std::string::npos)
            {
                auto get_vars = parse_argument_list(str.substr(get_begin, uri_end - get_begin));
                for(auto &c : get_vars)
                {
                    std::transform(c.first.begin(), c.first.end(), c.first.begin(), ::tolower);
                    get_data.emplace(std::move(c.first), std::move(c.second));
                }
                set_uri(str.substr(uri_begin, get_begin - uri_begin));
            }
            else
            {
                set_uri(str.substr(uri_begin, uri_end - uri_begin));
            }
            return;
        }
        throw std::invalid_argument("No URI found in: " + str);
    }
}