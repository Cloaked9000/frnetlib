//
// Created by fred on 10/12/16.
//

#include <algorithm>
#include "frnetlib/HttpRequest.h"

namespace fr
{
    HttpRequest::HttpRequest()
    : header_ended(false),
      last_parsed_character(0),
      content_length(0)
    {

    }

    bool HttpRequest::parse(const std::string &request)
    {
        body += request;

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
                if(!parse_header(header_end))
                    return false;
                body.clear();
            }

            body += request.substr(header_end + header_end_size, request.size() - header_end - header_end_size);
        }

        //If we've got the whole request, parse the POST if it exists
        if(body.size() >= content_length)
        {
            if(request_type == RequestType::Post)
                parse_post_body();
            return false;
        }

        return true;
    }

    bool HttpRequest::parse_header(int64_t header_end_pos)
    {
        try
        {
            //Split the header into lines
            size_t line = 0;
            std::vector<std::string> header_lines = split_string(body.substr(0, (unsigned long)header_end_pos));
            if(header_lines.empty())
                return false;

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
        std::string request = request_type_to_string(request_type == Http::Unknown ? Http::Get : request_type) + " " + uri + " HTTP/1.1\r\n";

        //Add the headers to the request
        for(const auto &header : header_data)
        {
            std::string data = header.first + ": " + header.second + "\r\n";
            request += data;
        }

        //Generate post line
        std::string post_string;
        for(auto &post : post_data)
            post_string += post.first + "=" + post.second + "&";
        if(!post_string.empty())
        {
            post_string.erase(request.size() - 1, 1);
            post_string += "\r\n";
        }

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
        auto post_begin = body.find_first_not_of("\r\n");
        if(post_begin == std::string::npos)
            post_begin = body.find_first_not_of("\n");
        if(post_begin != std::string::npos)
        {
            auto post = parse_argument_list(body.substr(post_begin, body.size() - post_begin));
            for(auto &c : post)
                post_data.emplace(std::move(c.first), std::move(c.second));
        }
    }

    void HttpRequest::parse_header_type(const std::string &str)
    {
        //Find the request type
        auto type_end = str.find(" ");
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
        auto uri_begin = str.find("/");
        auto uri_end = str.find("HTTP") - 1;
        if(uri_begin != std::string::npos)
        {
            //Extract URI
            std::string uri = str.substr(uri_begin, uri_end - uri_begin);

            //Parse GET variables
            auto get_begin = str.find("?");
            if(get_begin != std::string::npos)
            {
                auto get_vars = parse_argument_list(str.substr(get_begin, uri_end - get_begin));
                for(auto &c : get_vars)
                    get_data.emplace(std::move(c.first), std::move(c.second));
                uri.erase(get_begin, uri.size() - get_begin);
            }

            set_uri(uri);
            return;
        }
        throw std::invalid_argument("No URI found in: " + str);
    }
}