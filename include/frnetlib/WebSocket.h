//
// Created by fred on 01/03/18.
//

#ifndef FRNETLIB_WEBSOCKET_H
#define FRNETLIB_WEBSOCKET_H

#include <ctime>
#include "frnetlib/Socket.h"
#include "frnetlib/HttpRequest.h"
#include "frnetlib/HttpResponse.h"
#include "Base64.h"
#include "Sha1.h"
#include "WebFrame.h"

namespace fr
{
    template<typename SocketType = fr::Socket>
    class WebSocket : public SocketType
    {
    public:
        WebSocket()
        {}

        /*!
         * Connects the WebSocket to a WebSocket server. Makes
         * the connection using the underlying socket, and then handshakes.
         *
         * @param address The address of the socket to connect to
         * @param port The port of the socket to connect to
         * @param path The path of the resource
         * @param ws_protocols The list of supported Websocket-Protocols.
         * @param timeout The number of seconds to wait before timing the connection attempt out. Pass {} for default.
         * @return A Socket::Status indicating the status of the operation (Success on success, an error type on failure).
         */
        Socket::Status connect(const std::string &address, const std::string &port, const std::string &path,  const std::vector<std::string> &ws_protocols, std::chrono::seconds timeout)
        {
            //Establish a connection using the parent class
            Socket::Status status = SocketType::connect(address, port, timeout);
            if(status != Socket::Status::Success)
                return status;

            //Send an upgrade request header
            std::string websocket_key = Base64::encode(std::to_string(std::time(nullptr)));
            HttpRequest request;
            request.header("sec-websocket-key") = websocket_key;
            request.header("sec-websocket-version") = "13";

            if(!ws_protocols.empty()){
                request.header("sec-websocket-protocol") = ws_protocols[0];
                for(int i = 1;  i < ws_protocols.size(); i++)
                    request.header("sec-websocket-protocol") += ", " + ws_protocols[i];
            }

            request.header("connection") = "upgrade";
            request.header("upgrade") = "websocket";

            request.set_uri(path);

            status = SocketType::send(request);
            if(status != Socket::Status::Success)
                return status;

            //Receive the response
            HttpResponse response;
            status = SocketType::receive(response);
            if(status != Socket::Status::Success)
                return status;
            if(response.get_status() != Http::RequestStatus::SwitchingProtocols)
            {
                disconnect();
                errno = EPROTO;
                return Socket::Status::HandshakeFailed;
            }

            //Verify the sec-websocket-accept header
            std::string derived_key = response.header("sec-websocket-accept");
            if(derived_key != Base64::encode(Sha1::sha1_digest(websocket_key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11")))
            {
                disconnect();
                errno = EPROTO;
                return Socket::Status::HandshakeFailed;
            }

            return Socket::Status::Success;
        }

        /*!
         * Connects the WebSocket to a WebSocket server. Makes
         * the connection using the underlying socket, and then handshakes.
         *
         * @param address The address of the socket to connect to
         * @param port The port of the socket to connect to
         * @param timeout The number of seconds to wait before timing the connection attempt out. Pass {} for default.
         * @return A Socket::Status indicating the status of the operation (Success on success, an error type on failure).
         */
        Socket::Status connect(const std::string &address, const std::string &port, std::chrono::seconds timeout) override
        {
            return connect(address, port, "/", {}, timeout);
        }

        /*!
         * Sends a Disconnect Frame and then
         * closes the connection.
         */
        void disconnect() override
        {
            ClientWebFrame frame;
            frame.set_opcode(WebFrame::Opcode::Disconnect);
            if(SocketType::connected())
                SocketType::send(frame);
            SocketType::close_socket();
        }

        /*!
         * Sets the socket file descriptor. This is called by the Listener
         * when accepting a connection, and so we can use the opportunity to
         * handshake with the server.
         *
         * @param descriptor The socket descriptor.
         */
        void set_descriptor(void *descriptor) override
        {
            SocketType::set_descriptor(descriptor);
            if(!descriptor || SocketType::get_socket_descriptor() == -1)
                return;

            //Initialise connection, receive the handshake
            HttpRequest request;
            if(SocketType::receive(request) != Socket::Status::Success)
                throw std::runtime_error("Failed to receive WebSock handshake");

            if(request.header("Upgrade") != "websocket" || request.get_type() != Http::RequestType::Get)
                throw std::runtime_error("Client isn't using the WebSock protocol");

            //Calculate the derived key, then send back our response
            std::string derived_key = Base64::encode(Sha1::sha1_digest(request.header("sec-websocket-key") + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"));
            HttpResponse response;
            response.set_status(Http::RequestStatus::SwitchingProtocols);
            response.header("Upgrade") = "websocket";
            response.header("Connection") = "Upgrade";
            response.header("Sec-WebSocket-Accept") = derived_key;
            SocketType::send(response);
        }
    };
}

#endif //FRNETLIB_WEBSOCKET_H
