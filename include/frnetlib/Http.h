//
// Created by fred on 11/12/16.
//

#ifndef FRNETLIB_HTTP_H
#define FRNETLIB_HTTP_H
#include <string>
#include <vector>
#include <unordered_map>
#define HTTP_RECV_BUFFER_SIZE 8192

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
        virtual ~Http() = default;

        /*!
         * Parse a raw request or response from a string
         * into the object.
         *
         * @param data The request/response to parse
         * @return True if more data is needed, false if finished.
         */
        virtual bool parse(const std::string &data)=0;

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
        * Returns a reference to a header.
        * Can be used to either set/get the value.
        * If the key does not exist, then it will be
        * created and an empty value will be returned.
        *
        * @param key The name of the header
        * @return A reference to the header
        */
        std::string &header(std::string &&key);


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
         * Checks to see if a given header exists.
         * Note: 'key' should be lower case.
         *
         * @param key The name of the header
         * @return True if it does. False otherwise.
         */
        bool header_exists(const std::string &key) const;

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

    protected:
        /*!
         * Splits a string by new line. Ignores escaped \n's
         *
         * @return The split string
         */
        std::vector<std::string> split_string(const std::string &str);

        /*!
         * Converts a 'RequestType' enum value to
         * a printable string.
         *
         * @param type The RequestType to convert
         * @return The printable version of the enum value
         */
        std::string request_type_to_string(RequestType type) const;

        /*!
         * Converts hexadecimal to an integer.
         *
         * @param hex The hex value to convert
         * @return The decimal equivilent of the hexadecimal value.
         */
        static inline int dectohex(const std::string &hex)
        {
            return (int)strtol(&hex[0], 0, 16);
        }

        /*!
         * Converts a parameter list to a vector pair.
         * i.e: ?bob=10&fish=hey
         * to: <bob, 10>, <fish, hey>
         *
         * @param str The string to parse
         * @return The vector containing the results pairs
         */
        std::vector<std::pair<std::string, std::string>> parse_argument_list(const std::string &str);

        void parse_header_line(const std::string &str);

        //Other request info
        std::unordered_map<std::string, std::string> header_data;
        std::unordered_map<std::string, std::string> post_data;
        std::unordered_map<std::string, std::string> get_data;
        std::string body;
        RequestType request_type;
        std::string uri;
        RequestStatus status;
    };
}


#endif //FRNETLIB_HTTP_H
