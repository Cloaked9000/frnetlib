//
// Created by fred on 02/03/18.
//

#include <iostream>
#include <frnetlib/TcpListener.h>
#include <frnetlib/WebFrame.h>
#include <frnetlib/WebSocket.h>
#include <thread>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
int main()
{
    //Bind to port, fr::SSLListener should be used for TLS connections
    fr::TcpListener listener;
    if(listener.listen("9091") != fr::Socket::Success)
    {
        std::cerr << "Failed to bind to port!" << std::endl;
        return EXIT_FAILURE;
    }

    //Use fr::WebSocket<fr::SSLSocket> if using an SSL listener
    fr::WebSocket<fr::TcpSocket> socket;

    //Loop to send messages. Not ideal due to a lack of locking, but should be okay for an example.
    auto message_loop = [&socket]() {
        std::string message;
        fr::WebFrame frame;
        while(true)
        {
            frame.set_opcode(fr::WebFrame::Text);
            std::cout << "Message: " << std::endl;
            std::getline(std::cin, message);
            if(message == "exit")
            {
                socket.disconnect();
                return;
            }
            else if(message == "ping")
            {
                frame.set_opcode(fr::WebFrame::Ping);
            }
            frame.set_payload(std::move(message));
            socket.send(frame);
            message.clear(); //Returns message to a defined state after the std::move
        }
    };
    std::thread t1(message_loop);

    while(true)
    {
        //Accept a new WebSocket connection.
        if(listener.accept(socket) != fr::Socket::Success)
            continue;
        std::cout << "Accepted new connection: " << socket.get_remote_address() << std::endl;

        //Whilst we remain connected, keep processing messages
        while(socket.connected())
        {
            //Receive the next frame
            fr::WebFrame frame;
            if(socket.receive(frame) != fr::Socket::Success)
                continue;

            //If it's a Ping then we need to send back a frame containing the same payload, but of type Pong.
            if(frame.get_opcode() == fr::WebFrame::Ping)
            {
                std::cout << "Client sent a ping!" << std::endl;
                frame.set_opcode(fr::WebFrame::Pong);
                socket.send(frame);
                continue;
            }

            //If it's a disconnect message, then we should finish sending across any messages, and then call disconnect
            if(frame.get_opcode() == fr::WebFrame::Disconnect)
            {
                socket.disconnect();
                continue;
            }

            //The payload type could be Text, Binary, or Continuation.
            //If it's Continuation, then this frame is a part of a fragmented message, and it's
            //a continuation from a previous message. You can check if it's the final part of the
            //message using fr::WebFrame::is_final().
            std::cout << "Got a new message from the client. It's a ";
            if(frame.get_opcode() == fr::WebFrame::Text) std::cout << "text";
            else if(frame.get_opcode() == fr::WebFrame::Binary) std::cout << "binary";
            else if(frame.get_opcode() == fr::WebFrame::Pong) std::cout << "pong";
            else std::cout << "continuation of a previous";
            std::cout << " message. It is ";
            if(!frame.is_final()) std::cout << "not ";
            std::cout << "the final part of the message. The payload contains: '" << frame.get_payload() << "'." << std::endl;
        }
        std::cout << socket.get_remote_address() << " disconnected!" << std::endl;
    }
}
#pragma clang diagnostic pop