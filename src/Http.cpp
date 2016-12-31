//
// Created by fred on 11/12/16.
//

#include <iostream>
#include <sstream>
#include "frnetlib/Http.h"

namespace fr
{
    const static std::string request_type_strings[Http::RequestType::RequestTypeCount] = {"UNKNOWN", "GET", "POST"};

    Http::Http()
    {
        clear();
    }

    Http::RequestType Http::get_type() const
    {
        return request_type;
    }

    std::string &Http::operator[](const std::string &key)
    {
        return headers[key];
    }

    std::vector<std::string> Http::split_string(const std::string &str)
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
        result.emplace_back(str.substr(line_start, str.size() - line_start));
        return result;
    }

    void Http::set_body(const std::string &body_)
    {
        body = body_;
    }

    void Http::clear()
    {
        headers.clear();
        body.clear();
        get_variables.clear();
        uri = "/";
        status = Ok;
        request_type = Unknown;
    }

    std::string &Http::get(const std::string &key)
    {
        return get_variables[key];
    }

    std::string &Http::post(const std::string &key)
    {
        return headers[key];
    }

    bool Http::get_exists(const std::string &key) const
    {
        return get_variables.find(key) != get_variables.end();
    }

    bool Http::post_exists(const std::string &key) const
    {
        return headers.find(key) != headers.end();
    }

    const std::string &Http::get_uri() const
    {
        return uri;
    }

    void Http::set_status(RequestStatus status_)
    {
        status = status_;
    }

    Http::RequestStatus Http::get_status() const
    {
        return status;
    }

    void Http::set_uri(const std::string &str)
    {
        uri = str;
    }

    std::string Http::request_type_to_string(RequestType type) const
    {
        if(type >= RequestType::RequestTypeCount)
            return request_type_strings[0];
        return request_type_strings[type];
    }

    void Http::set_type(Http::RequestType type)
    {
        request_type = type;
    }

    const std::string &Http::get_body() const
    {
        return body;
    }

    std::string Http::url_encode(const std::string &str)
    {
        std::stringstream encoded;
        encoded << std::hex;
        for(const auto &c : str)
        {
            if(isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
                encoded << c;
            else if(c == ' ')
                encoded << '+';
            else
                encoded << "%" << std::uppercase << (int)c << std::nouppercase;
        }
        return encoded.str();
    }

    std::string Http::url_decode(const std::string &str)
    {
        std::string result;
        for(size_t a = 0; a < str.size(); a++)
        {
            if(str[a] == '%' && a < str.size() - 1)
            {
                result += (char)dectohex(str.substr(a + 1, 2));
                a += 2;
            }
            else if(str[a] == '+')
            {
                result += " ";
            }
            else
            {
                result += str[a];
            }
        }
        return result;
    }
}