//
// Created by fred.nicolson on 23/05/17.
//

#ifndef FRNETLIB_URLPARSER_H
#define FRNETLIB_URLPARSER_H
#include <string>
#include <unordered_map>

//Note, a URL looks like this:  scheme:[//host[:port]][/path][?query][#fragment]
namespace fr
{
    class URL
    {
    public:
        enum Scheme
        {
            HTTP = 0,
            HTTPS = 1,
            FTP = 2,
            MAILTO = 3,
            IRC = 4,
            SFTP = 5,
            Unknown = 6,
        };

        /*!
         * Constructors
         */
        URL() = default;

        URL(const std::string &url);

        /*!
         * Parses a given URL, extracting its various components
         *
         * @param url The URL to parse
         */
        void parse(std::string url);

        /*!
         * Get the URL scheme
         */
        inline Scheme get_scheme() const
        {
            return scheme;
        }

        /*
         * Get the URL host
         */
        inline const std::string &get_host() const
        {
            return host;
        }

        /*
         * Get the URL port
         */
        inline const std::string &get_port() const
        {
            return port;
        }

        /*
         * Get the URL path
         */
        inline const std::string &get_path() const
        {
            return path;
        }

        /*
         * Get the URL query
         */
        inline const std::string &get_query() const
        {
            return query;
        }

        /*
         * Get the URL fragment
         */
        inline const std::string &get_fragment() const
        {
            return fragment;
        }

        /*!
         * Converts a string to a scheme enum.
         *
         * @param scheme The string scheme to convert
         * @return The associated scheme enum value. Scheme::Unknown on failure.
         */
        static Scheme string_to_scheme(const std::string &scheme);

        /*!
         * Converts a scheme enum value to a string
         *
         * @param scheme The scheme value to convert
         * @return The string version
         */
        static const std::string &scheme_to_string(Scheme scheme);

    private:
        /*!
         * Converts a string to lower case
         *
         * @param str The string to convert
         * @return The converted string
         */
        static std::string to_lower(const std::string &str);

        //State
        Scheme scheme;
        std::string host;
        std::string port;
        std::string path;
        std::string query;
        std::string fragment;
        static std::unordered_map<std::string, URL::Scheme> scheme_string_map;
    };
}

#endif //FRNETLIB_URLPARSER_H
