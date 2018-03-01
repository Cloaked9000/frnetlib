//
// Created by fred on 11/12/16.
//

#ifndef FRNETLIB_HTTP_H
#define FRNETLIB_HTTP_H
#include <string>
#include <vector>
#include <unordered_map>
#include "Socket.h"
#include "Sendable.h"

namespace fr
{
    class Http : public Sendable
    {
    public:
        enum RequestType
        {
            Get = 0,
            Post = 1,
            Put = 2,
            Delete = 3,
            Patch = 4,
            RequestTypeCount = 5, //Keep me at the end of valid HTTP request types, and updated
            Unknown = 6,
            Partial = 7,
        };
        enum RequestStatus
        {
            Continue = 100,
            SwitchingProtocols = 101,
            Ok = 200,
            Created = 201,
            Accepted = 202,
            NonAuthorativeInformation = 203,
            NoContent = 204,
            ResetContent = 205,
            PartialContent = 206,
            IMUsed = 226,
            MultipleChoices = 300,
            MovedPermanently = 301,
            Found = 302,
            SeeOther = 303,
            NotModified = 304,
            UseProxy = 305,
            TemporaryRedirect = 307,
            PermanentRedirect = 308,
            BadRequest = 400,
            Unauthorised = 401,
            PaymentRequired = 402,
            Forbidden = 403,
            NotFound = 404,
            MethodNotAllowed = 405,
            NotAcceptable = 406,
            ProxyAuthenticationRequired = 407,
            RequestTimeout = 408,
            Conflict = 409,
            Gone = 410,
            LengthRequired = 411,
            PreconditionFailed = 412,
            RequestEntityTooLarge = 413,
            RequestURITooLong = 414,
            UnsupportedMediaType = 415,
            RequestedRangeNotSatisfiable = 416,
            ExpectationFailed = 417,
            ImATeapot = 418,
            UnprocessableEntity = 422,
            UpgradeRequired = 426,
            PreconditionRequired = 428,
            TooManyRequests = 429,
            RequestHeaderFieldTooLarge = 431,
            UnvailableForLegalReasons = 451,
            InternalServerError = 500,
            NotImplemented = 501,
            BadGateway = 502,
            ServiceUnavailable = 503,
            GatewayTimeout = 504,
            HTTPVersionNotSupported = 505,
            VariantAlsoNegotiates = 506,
            NotExtended = 510,
            NetworkAuthenticationRequired = 511,
            NetworkReadTimeoutError = 598,
            NetworkConnectTimeoutError = 599,
        };

        Http();
        Http(Http&&)=default;
        Http(const Http&)= default;
        Http &operator=(const Http &)=default;
        Http &operator=(Http &&)=default;
        virtual ~Http() = default;


        /*!
         * Parse a raw request or response from a string
         * into the object.
         *
         * @param data The request/response to parse
         * @param datasz The length of data in bytes
         * @return NotEnoughData if parse needs to be called again. Success on success, other on error.
         */
        virtual fr::Socket::Status parse(const char *data, size_t datasz)=0;

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
         * Sets the request body
         *
         * @param body_ The request body
         */
        void set_body(const std::string &body_);

        /*!
         * Returns a reference to a get variable.
         * Can be used to either set/get the value.
         * If the key does not exist, then it will be
         * created and an empty value will be returned.
         *
         * @param key The name of the GET variable
         * @return A reference to the GET variable
         */
        std::string &get(std::string key);

        /*!
        * Returns a reference to a POST variable.
        * Can be used to either set/get the value.
        * If the key does not exist, then it will be
        * created and an empty value will be returned.
        *
        * @param key The name of the POST variable
        * @return A reference to the POST variable
        */
        std::string &post(std::string key);

        /*!
        * Returns a reference to a header.
        * Can be used to either set/get the value.
        * If the key does not exist, then it will be
        * created and an empty value will be returned.
        *
        * @param key The name of the header
        * @return A reference to the header
        */
        std::string &header(std::string key);


        /*!
         * Checks to see if a given GET variable exists
         *
         * @param key The name of the GET variable
         * @return True if it does. False otherwise.
         */
        bool get_exists(std::string key) const;

        /*!
         * Checks to see if a given POST variable exists
         *
         * @param key The name of the POST variable
         * @return True if it does. False otherwise.
         */
        bool post_exists(std::string key) const;

        /*!
         * Checks to see if a given header exists.
         *
         * @param key The name of the header
         * @return True if it does. False otherwise.
         */
        bool header_exists(std::string key) const;

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

        /*!
         * URL Encodes a given string
         *
         * @param str The string to URL encode
         * @return The URL encoded string
         */
        static std::string url_encode(const std::string &str);

        /*!
         * Decodes a URL encoded string.
         *
         * @param str The string to decode
         * @return The decoded string
         */
        static std::string url_decode(const std::string &str);

        /*!
         * Gets the mimetype of a given filename, or file extention.
         *
         * @param filename The filename, or file extention to find the mime type of
         * @return The mime type. Returns 'application/octet-stream' if it couldn't be found.
         */
        const static std::string &get_mimetype(const std::string &filename);

        /*!
         * Converts a 'RequestType' enum value to
         * a printable string.
         *
         * @param type The RequestType to convert
         * @return The printable version of the enum value
         */
        static std::string request_type_to_string(RequestType type);

        /*!
         * Converts a string value into a 'RequestType' enum value.
         *
         * @param str The string to convert
         * @return The converted RequestType. Unknown on failure. Or Partial if str is part of a request type.
         */
        static RequestType string_to_request_type(const std::string &str) ;

    protected:
        /*!
         * Splits a string by new line. Ignores escaped \n's
         *
         * @return The split string
         */
        static std::vector<std::string> split_string(const std::string &str);

        /*!
         * Converts a parameter list to a vector pair.
         * i.e: ?bob=10&fish=hey
         * to: <bob, 10>, <fish, hey>
         *
         * @param str The string to parse
         * @return The vector containing the results pairs
         */
        static std::vector<std::pair<std::string, std::string>> parse_argument_list(const std::string &str);

        /*!
         * Parses a header line in a HTTP request/response
         *
         * @param str The header. E.g: header: value
         */
        void parse_header_line(const std::string &str);

        /*!
         * Overridable send, to allow
         * custom types to be directly sent through
         * sockets.
         *
         * @param socket The socket to send through
         * @return Status indicating if the send succeeded or not.
         */
        virtual Socket::Status send(Socket *socket) override;

        /*!
         * Overrideable receive, to allow
         * custom types to be directly received through
         * sockets.
         *
         * @param socket The socket to send through
         * @return Status indicating if the send succeeded or not.
         */
        virtual Socket::Status receive(Socket *socket) override;

        //Other request info
        std::unordered_map<std::string, std::string> header_data;
        std::unordered_map<std::string, std::string> post_data;
        std::unordered_map<std::string, std::string> get_data;
        std::string body;
        RequestType request_type;
        std::string uri;
        RequestStatus status;

    private:

    };
}


#endif //FRNETLIB_HTTP_H
