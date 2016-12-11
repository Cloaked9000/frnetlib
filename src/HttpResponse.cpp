//
// Created by fred on 10/12/16.
//

#include <iostream>
#include "HttpResponse.h"

namespace fr
{
    void HttpResponse::parse(const std::string &response_data)
    {
        std::cout << "Parsing: " << response_data << std::endl;
        //Clear old headers/data
        clear();

        //Make sure there's actual request data to read
        if(response_data.empty())
            return;

        //Split by new lines
        std::vector<std::string> lines = split_string(response_data);
        if(lines.empty())
            return;

        //Extract request get_type
        if(lines[0].find("GET") != std::string::npos)
            request_type = RequestType::Get;
        else if(lines[0].find("POST") != std::string::npos)
            request_type = RequestType::Post;
        else
            request_type = RequestType::Unknown;

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

    std::string HttpResponse::construct(const std::string &host) const
    {
        //Add HTTP header
        std::string response = "HTTP/1.1 " + std::to_string(status) + " \r\n";

        //Add the headers to the response
        for(const auto &header : headers)
        {
            std::string data = header.first + ": " + header.second + "\r\n";
            response += data;
        }

        //Add in required headers if they're missing
        if(headers.find("Connection") == headers.end())
            response += "Connection: close\r\n";
        if(headers.find("Content-type") == headers.end())
            response += "Content-type: text/html\r\n";

        //Add in space
        response += "\r\n";

        //Add in the body
        response += body + "\r\n";
        return response;
    }
}