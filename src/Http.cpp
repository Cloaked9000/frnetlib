//
// Created by fred on 11/12/16.
//

#include <iostream>
#include <sstream>
#include <algorithm>
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
        post_data.clear();
        get_data.clear();
        post_data.clear();
        body.clear();
        uri = "/";
        status = Ok;
        request_type = Unknown;
    }

    std::string &Http::get(const std::string &key)
    {
        return get_data[key];
    }

    std::string &Http::post(const std::string &key)
    {
        return post_data[key];
    }

    bool Http::get_exists(const std::string &key) const
    {
        return get_data.find(key) != get_data.end();
    }

    bool Http::post_exists(const std::string &key) const
    {
        return post_data.find(key) != post_data.end();
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

    std::string &Http::header(const std::string &key)
    {
        return header_data[key];
    }

    bool Http::header_exists(const std::string &key) const
    {
        return header_data.find(key) != header_data.end();
    }

    std::vector<std::pair<std::string, std::string>> Http::parse_argument_list(const std::string &str)
    {
        std::vector<std::pair<std::string, std::string>> list;
        if(str.empty())
            return list;

        size_t read_index = 0;
        if(str.front() == '?')
            read_index++;

        while(true)
        {
            auto equal_pos = str.find("=", read_index);
            if(equal_pos != std::string::npos)
            {
                auto and_pos = str.find("&", read_index);
                if(and_pos == std::string::npos)
                {
                    list.emplace_back(str.substr(read_index, equal_pos - read_index), str.substr(equal_pos + 1, str.size() - equal_pos - 1));
                    break;
                }
                else
                {
                    list.emplace_back(str.substr(read_index, equal_pos - read_index), str.substr(equal_pos + 1, and_pos - equal_pos - 1));
                    read_index = and_pos + 1;
                }
            }
            else
            {
                break;
            }
        }

        return list;
    }

    void Http::parse_header_line(const std::string &str)
    {
        auto colon_pos = str.find(":");
        if(colon_pos != std::string::npos)
        {
            auto data_begin = str.find_first_not_of(" ", colon_pos + 1);
            if(data_begin != std::string::npos)
            {
                std::string header_name = str.substr(0, colon_pos);
                std::transform(header_name.begin(), header_name.end(), header_name.begin(), ::tolower);
                header_data.emplace(std::move(header_name), str.substr(data_begin, str.size() - (data_begin + 1)));
            }
        }
    }
}