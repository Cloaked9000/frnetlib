//
// Created by fred on 10/12/16.
//

#include "HttpRequest.h"

namespace fr
{
    const static std::string request_type_strings[HttpRequest::RequestType::RequestTypeCount] = {"UNKNOWN", "GET", "POST"};

    HttpRequest::HttpRequest()
    {
        clear();
    }

    void HttpRequest::parse_request(const std::string &request_data)
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

    void HttpRequest::parse_response(const std::string &response_data)
    {
        //Warning: Horrible string parsing code
        std::cout << "Parsing response: " << std::endl << response_data << std::endl;
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

    HttpRequest::RequestType HttpRequest::get_type() const
    {
        return request_type;
    }

    std::string &HttpRequest::operator[](const std::string &key)
    {
        return headers[key];
    }

    std::vector<std::string> HttpRequest::split_string(const std::string &str)
    {
        char last_character = '\0';
        size_t line_start = 0;
        std::vector<std::string> result;

        for(size_t a = 0; a < str.size(); a++)
        {
            if(str[a] == '\n' && last_character != '\\')
            {
                result.emplace_back(str.substr(line_start, a - line_start));
                line_start = a + 1;
            }
            last_character = str[a];
        }
        return result;
    }

    std::string HttpRequest::construct_request() const
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

    std::string HttpRequest::construct_response() const
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

    void HttpRequest::set_body(const std::string &body_)
    {
        body = body_;
    }

    void HttpRequest::clear()
    {
        headers.clear();
        body.clear();
        get_variables.clear();
        uri = "/";
        status = Ok;
        request_type = Get;
    }

    std::string &HttpRequest::get(const std::string &key)
    {
        return get_variables[key];
    }

    std::string &HttpRequest::post(const std::string &key)
    {
        return headers[key];
    }

    bool HttpRequest::get_exists(const std::string &key) const
    {
        return get_variables.find(key) != get_variables.end();
    }

    bool HttpRequest::post_exists(const std::string &key) const
    {
        return headers.find(key) != headers.end();
    }

    const std::string &HttpRequest::get_uri() const
    {
        return uri;
    }

    void HttpRequest::set_status(RequestStatus status_)
    {
        status = status_;
    }

    HttpRequest::RequestStatus HttpRequest::get_status()
    {
        return status;
    }

    void HttpRequest::set_uri(const std::string &str)
    {
        uri = str;
    }

    std::string HttpRequest::request_type_to_string(RequestType type) const
    {
        if(type >= RequestType::RequestTypeCount)
            return request_type_strings[0];
        return request_type_strings[type];
    }

    void HttpRequest::set_type(HttpRequest::RequestType type)
    {
        request_type = type;
    }

    const std::string &HttpRequest::get_body() const
    {
        return body;
    }
}