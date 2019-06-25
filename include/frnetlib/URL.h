//
// Created by fred.nicolson on 23/05/17.
//

#ifndef FRNETLIB_URLPARSER_H
#define FRNETLIB_URLPARSER_H
#include <string>
#include <unordered_map>
#include <algorithm>

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

        explicit URL(const std::string &url);

        /*!
         * Parses a given URL, extracting its various components. This will always
         * try to make something out of the provided URL. It will also try its best
         * to guess any potentially missing information. For example, if the URL is
         * https://example.com, then the 'port' will be guessed as 443.
         *
         * @param url The URL to parse
         */
        void parse(const std::string &url);

        /*!
         * Get the URL scheme
         */
        inline Scheme get_scheme() const
        {
            return scheme;
        }

        /*
         * Set the URL host
         */
        inline void set_scheme(Scheme scheme_)
        {
            scheme = scheme_;
        }

        /*
         * Get the URL host
         */
        inline const std::string &get_host() const
        {
            return host;
        }

        /*
         * Set the URL host
         */
        inline void set_host(std::string host_)
        {
            host = std::move(host_);
        }

        /*
         * Get the URL port
         */
        inline const std::string &get_port() const
        {
            return port;
        }

        /*!
         * Set the port
         */
        inline void set_port(std::string port_)
        {
            port = std::move(port_);
        }

        /*
         * Get the URL path
         */
        inline const std::string &get_path() const
        {
            return path;
        }

        /*!
         * Set the path
         */
        inline void set_path(std::string path_)
        {
            path = std::move(path_);
        }


        /*
         * Get the URL query
         */
        inline const std::string &get_query() const
        {
            return query;
        }

        /*!
         * Set the query
         */
        inline void set_query(std::string query_)
        {
            query = std::move(query_);
        }

        /*
         * Get the URL fragment
         */
        inline const std::string &get_fragment() const
        {
            return fragment;
        }

        /*!
         * Set the fragment
         */
        inline void set_fragment(std::string fragment_)
        {
            fragment = std::move(fragment_);
        }

        /*!
         * Returns the combination of other URL elements into a single URI string.
         *
         * The URL is everything after the IP/Address & Port.
         *
         * @return The URL's URI
         */
        inline std::string get_uri() const
        {
            std::string result;
            result += get_path();
            if(!get_query().empty())
                result += "?" + get_query();
            if(!get_fragment().empty())
                result += "#" + get_fragment();
            return result;
        }

        /*!
         * Gets the *whole* URL, including every single element
         *
         * @return Whole URL
         */
        inline std::string get_url() const
        {
            std::string ret;
            if(!get_host().empty())
            {
                if(scheme != Scheme::Unknown)
                    ret.append(scheme_to_string(scheme)).append("://");
                ret.append(get_host());
                if(!get_port().empty())
                    ret.append(":").append(port);
            }
            ret.append(get_uri());
            return ret;
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

        /*!
         * Comparison operator
         *
         * @param o Object to compare against
         * @return True if equal, false otherwise
         */
        inline bool operator==(const URL &o) const
        {
            return scheme == o.scheme && host == o.host && port == o.port && path == o.path && query == o.query && fragment == o.fragment;
        }
    private:
        /*!
         * Converts a string to lower case
         *
         * @param str The string to convert
         * @return The converted string
         */
        static std::string to_lower(const std::string &str)
        {
            std::string out = str;
            std::transform(out.begin(), out.end(), out.begin(), ::tolower);
            return out;
        }

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
