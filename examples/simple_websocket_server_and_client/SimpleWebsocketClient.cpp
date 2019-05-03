//
// Created by fred on 02/03/18.
//

#include <thread>
#include <iostream>
#include <frnetlib/WebFrame.h>
#include <frnetlib/WebSocket.h>
#include <frnetlib/TcpSocket.h>

int main()
{
    //Connect to the WebSocket server
    fr::WebSocket<fr::TcpSocket> socket; //Use an fr::SSLSocket for secure connections
    if(socket.connect("127.0.0.1", "9091", {}) != fr::Socket::Status::Success)
    {
        std::cerr << "Failed to connect to server!" << std::endl;
        return EXIT_FAILURE;
    }

    //Loop to send messages. Not ideal due to a lack of locking, but should be okay for an example.
    auto message_loop = [&socket]() {
        std::string message;
        fr::WebFrame frame;
        while(socket.connected())
        {
            std::cout << "Message: " << std::endl;
            std::getline(std::cin, message);
            if(message == "exit")
            {
                socket.disconnect();
                return;
            }
            else if(message == "ping")
            {
                frame.set_opcode(fr::WebFrame::Opcode::Ping);
            }
            frame.set_payload(std::move(message));
            socket.send(frame);
            message.clear(); //Returns message to a defined state after the std::move
        }
    };
    std::thread t1(message_loop);

    //Put the socket into non-blocking mode, for dealing with PINGs and whatnot without waiting.
    while(socket.connected())
    {
        //Receive the next frame
        fr::WebFrame frame;
        if(socket.receive(frame) != fr::Socket::Status::Success)
            continue;

        //If it's a Ping then we need to send back a frame containing the same payload, but of type Pong.
        if(frame.get_opcode() == fr::WebFrame::Opcode::Ping)
        {
            std::cout << "Server sent a ping!" << std::endl;
            frame.set_opcode(fr::WebFrame::Opcode::Pong);
            socket.send(frame);
            continue;
        }

        //If it's a disconnect message, then we should finish sending across any messages, and then call disconnect
        if(frame.get_opcode() == fr::WebFrame::Opcode::Disconnect)
        {
            socket.disconnect();
            continue;
        }

        //The payload type could be Text, Binary, or Continuation.
        //If it's Continuation, then this frame is a part of a fragmented message, and it's
        //a continuation from a previous message. You can check if it's the final part of the
        //message using fr::WebFrame::is_final().
        std::cout << "Got a new message from the server. It's a ";
        if(frame.get_opcode() == fr::WebFrame::Opcode::Text) std::cout << "text";
        else if(frame.get_opcode() == fr::WebFrame::Opcode::Binary) std::cout << "binary";
        else if(frame.get_opcode() == fr::WebFrame::Opcode::Pong) std::cout << "pong";
        else std::cout << "continuation of a previous";
        std::cout << " message. It is ";
        if(!frame.is_final()) std::cout << "not ";
        std::cout << "the final part of the message. The payload contains: '" << frame.get_payload() << "'." << std::endl;
    }
    std::cout << "The server disconnected!" << std::endl;
    t1.join();
}