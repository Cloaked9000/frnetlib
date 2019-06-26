//
// Created by fred.nicolson on 23/05/17.
//

#include <stdexcept>
#include <algorithm>
#include <iostream>
#include "frnetlib/URL.h"

namespace fr
{

    std::unordered_map<std::string, URL::Scheme> URL::scheme_string_map = {
            {"http",    URL::HTTP},
            {"https",   URL::HTTPS},
            {"sftp",    URL::FTP},
            {"mailto",  URL::MAILTO},
            {"irc",     URL::IRC},
            {"sftp",    URL::SFTP},
            {"unknown", URL::Unknown}
    };

    URL::URL(const std::string &url)
    {
        parse(url);
    }


    void URL::parse(const std::string &url)
    {
        scheme = Scheme::Unknown;
        size_t parse_offset = 0;
        size_t pos = 0;

        //Check to see if a scheme exists
        pos = url.find("://");
        if(pos != std::string::npos)
        {
            auto scheme_pos = scheme_string_map.find(to_lower(url.substr(0, pos)));
            scheme = (scheme_pos == scheme_string_map.end()) ? URL::Unknown : scheme_pos->second;
            parse_offset = pos + 3; // skip the ://
        }

        //Check to see if there's a port
        pos = url.find(':', parse_offset);
        if(pos != std::string::npos) //A port is provided
        {
            //Store host
            host = url.substr(parse_offset, pos - parse_offset);
            parse_offset += host.size();

            //Find end of port
            size_t port_end = url.find('/', parse_offset);
            port_end = (port_end == std::string::npos) ? url.size() : port_end;
            port = url.substr(pos + 1, port_end - pos - 1);
            parse_offset = port_end + 1;
        }
        else //There's no port
        {
            //Store host
            pos = url.find('/', parse_offset);
            pos = (pos != std::string::npos) ? pos : url.find('?', parse_offset);
            pos = (pos != std::string::npos) ? pos : url.find('#', parse_offset);
            if(pos != std::string::npos || parse_offset != 0)
            {
                pos = (pos != std::string::npos) ? pos : url.size();
                host = url.substr(parse_offset, pos - parse_offset);
                parse_offset = pos + 1;
            }

            //Guess port based on scheme, if it's not explicitly provided.
            switch(scheme)
            {
                case URL::HTTP:
                    port = "80";
                    break;
                case URL::HTTPS:
                    port = "443";
                    break;
                case URL::IRC:
                    port = "6697";
                    break;
                case URL::FTP:
                    port = "21";
                    break;
                case URL::SFTP:
                    port = "25";
                    break;
                default:
                    break;
            }
        }

        //Exit if done
        if(parse_offset >= url.size())
            return;

        //Extract the path
        pos = url.find('?', parse_offset);
        if(pos != std::string::npos)
        {
            path.append("/").append(url.substr(parse_offset, pos - parse_offset));
            parse_offset = pos + 1;
        }
        else
        {
            pos = url.find('#', parse_offset);
            pos = (pos != std::string::npos) ? pos : url.find('?', parse_offset);
            pos = (pos != std::string::npos) ? pos : url.size();
            path.append("/").append(url.substr(parse_offset, pos - parse_offset));
            parse_offset = pos + 1;
        }

        //Extract the query
        pos = url.find('#', parse_offset - 1);
        if(pos != std::string::npos)
        {
            if(pos + 1 != parse_offset)
                query = url.substr(parse_offset, pos - parse_offset);
            fragment = url.substr(pos + 1, url.size() - pos - 1);
        }
        else
        {
            if(parse_offset >= url.size())
                return;
            query = url.substr(parse_offset, url.size() - parse_offset);
        }

    }

    URL::Scheme URL::string_to_scheme(const std::string &scheme)
    {
        auto iter = scheme_string_map.find(to_lower(scheme));
        if(iter == scheme_string_map.end())
            return URL::Unknown;
        return iter->second;
    }

    const std::string &URL::scheme_to_string(URL::Scheme scheme)
    {
        auto iter = std::find_if(scheme_string_map.begin(), scheme_string_map.end(), [&](const auto &i)
        {
            return i.second == scheme;
        });

        if(iter == scheme_string_map.end())
            throw std::logic_error("Unknown URL::Scheme value " + std::to_string(scheme));
        return iter->first;
    }
}