//
// Created by fred on 10/12/16.
//

#include "HttpRequest.h"

namespace fr
{
    void HttpRequest::parse(const std::string &request_data)
    {
        //Warning: Horrible string parsing code

        //Clear old headers/data
        clear();

        //Make sure there's actual request data to read
        if(request_data.empty())
            return;

        //Split by new lines
        std::vector<std::string> lines = split_string(request_data);
        if(lines.empty())
            return;

        //Extract request get_type
        if(lines[0].find("GET") != std::string::npos)
            request_type = RequestType::Get;
        else if(lines[0].find("POST") != std::string::npos)
            request_type = RequestType::Post;
        else
            request_type = RequestType::Unknown;

        //Remove HTTP version
        auto http_version = lines[0].find("HTTP");
        if(http_version != std::string::npos && http_version > 0)
            lines[0].erase(http_version - 1, lines[0].size() - http_version + 1);

        //Extract URI & GET variables
        auto uri_start = lines[0].find(" ");
        auto uri_end = lines[0].find("?");
        if(uri_start != std::string::npos)
        {
            if(uri_end == std::string::npos) //If no GET arguments
            {
                uri = lines[0].substr(uri_start + 1, lines[0].size() - 1);
            }
            else //There's get arguments
            {
                uri = lines[0].substr(uri_start + 1, uri_end - uri_start - 1);
                std::string get_lines = lines[0].substr(uri_end + 1, lines[0].size());
                std::string name_buffer, value_buffer;

                bool state = false;
                for(size_t a = 0; a < get_lines.size(); a++)
                {
                    if(get_lines[a] == '&')
                    {
                        get_variables.emplace(name_buffer, value_buffer);
                        name_buffer.clear();
                        value_buffer.clear();
                        state = false;
                        continue;
                    }
                    else if(get_lines[a] == '=')
                    {
                        state = true;
                    }
                    else if(state)
                    {
                        value_buffer += get_lines[a];
                    }
                    else
                    {
                        name_buffer += get_lines[a];
                    }
                }
                get_variables.emplace(name_buffer, value_buffer);
            }
        }

        //Extract headers
        size_t a;
        for(a = 1; a < lines.size(); a++)
        {
            //New line indicates headers have ended
            if(lines[a].empty() || lines[a].size() <= 2)
                break;

            //Find the colon separating the header name and header data
            auto colon_iter = lines[a].find(":");
            if(colon_iter == std::string::npos)
                continue;

            //Store the header
            std::string header_name = lines[a].substr(0, colon_iter);
            std::string header_content = lines[a].substr(colon_iter + 2, lines[a].size () - colon_iter - 3);
            headers.emplace(header_name, header_content);
        }

        //Store request body
        for(; a < lines.size(); a++)
        {
            body += lines[a] + "\n";
        }
        return;
    }

    std::string HttpRequest::construct() const
    {
        //Add HTTP header
        std::string request = request_type_to_string(request_type) + " " + uri + " HTTP/1.1\r\n";

        //Add the headers to the request
        for(const auto &header : headers)
        {
            std::string data = header.first + ": " + header.second + "\r\n";
            request += data;
        }

        //Add in required headers if they're missing
        if(headers.find("Connection") == headers.end())
            request += "Connection: keep-alive\r\n";

        //Add in space
        request += "\r\n";

        //Add in the body
        request += body + "\r\n";
        return request;
    }
}