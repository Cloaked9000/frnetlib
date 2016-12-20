//
// Created by fred on 11/12/16.
//

#ifndef FRNETLIB_HTTP_H
#define FRNETLIB_HTTP_H
#include <string>
#include <vector>
#include <unordered_map>

namespace fr
{
    class Http
    {
    public:
        enum RequestType
        {
            Unknown = 0,
            Get = 1,
            Post = 2,
            RequestTypeCount = 3,
        };
        enum RequestStatus
        {
            Ok = 200,
            BadRequest = 400,
            Forbidden = 403,
            NotFound = 404,
            ImATeapot = 418,
            InternalServerError = 500,
        };

        Http();

        /*!
         * Parse a raw request or response from a string
         * into the object.
         *
         * @param data The request/response to parse
         */
        virtual void parse(const std::string &data)=0;

        /*!
         * Constructs a HTTP request/response to send.
         *
         * @param host The host that we're connected to.
         * @return The HTTP request
         */
        virtual std::string construct(const std::string &host) const=0;

        /*!
         * Gets the request type (post, get etc)
         *
         * @return The request type
         */
        RequestType get_type() const;

        /*!
         * Sets the request type (post, get etc)
         *
         * @param type The type of request to set it to
         */
        void set_type(RequestType type);

        /*!
         * Access a header
         *
         * @param key The name of the header data to access/create
         * @return The header data.
         */
        std::string &operator[](const std::string &key);

        /*!
         * Sets the request body
         *
         * @param body_ The request body
         */
        void set_body(const std::string &body_);

        /*!
         * Clear current request data
         */
        void clear();

        /*!
         * Returns a reference to a get variable.
         * Can be used to either set/get the value.
         * If the key does not exist, then it will be
         * created and an empty value will be returned.
         *
         * @param key The name of the GET variable
         * @return A reference to the GET variable
         */
        std::string &get(const std::string &key);

        /*!
        * Returns a reference to a POST variable.
        * Can be used to either set/get the value.
        * If the key does not exist, then it will be
        * created and an empty value will be returned.
        *
        * @param key The name of the POST variable
        * @return A reference to the POST variable
        */
        std::string &post(const std::string &key);

        /*!
         * Checks to see if a given GET variable exists
         *
         * @param key The name of the GET variable
         * @return True if it does. False otherwise.
         */
        bool get_exists(const std::string &key) const;

        /*!
         * Checks to see if a given POST variable exists
         *
         * @param key The name of the POST variable
         * @return True if it does. False otherwise.
         */
        bool post_exists(const std::string &key) const;

        /*!
         * Returns the requested URI
         *
         * @return The URI
         */
        const std::string &get_uri() const;

        /*!
         * Sets the response status (400, 200, etc)
         *
         * @param status The status to send back
         */
        void set_status(RequestStatus status);

        /*!
         * Gets the reponse status
         *
         * @return The status
         */
        RequestStatus get_status() const;

        /*!
         * Sets the request URI.
         *
         * @param str What to set the URI to.
         */
        void set_uri(const std::string &str);

        /*!
         * Gets the body of the HTTP request
         *
         * @return The request body
         */
        const std::string &get_body() const;

    protected:
        /*!
         * Splits a string by new line. Ignores escaped \n's
         *
         * @return The split string
         */
        std::vector<std::string> split_string(const std::string &str);

        std::string request_type_to_string(RequestType type) const;

        //Other request info
        std::unordered_map<std::string, std::string> headers;
        std::unordered_map<std::string, std::string> get_variables;
        std::string body;
        RequestType request_type;
        std::string uri;
        RequestStatus status;
    };
}


#endif //FRNETLIB_HTTP_H