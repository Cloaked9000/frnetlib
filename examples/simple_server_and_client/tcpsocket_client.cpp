#include <iostream>
#include <frnetlib/Packet.h>
#include <frnetlib/TcpSocket.h>
#include <frnetlib/TcpListener.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT "8081"


int send_a_packet(fr::TcpSocket &socket)
{
    std::cout << "Going to send something..." << std::endl;

    //Send the request
    fr::Packet packet;
    packet << "Hello there, I am " << (float)1.2 << " years old";
    if(socket.send(packet) != fr::Socket::Success)
    {
        //Failed to send packet
        std::cout << "Seems got something wrong when sending" << std::endl;
        return -1;
    }

    //Receive a response
    std::cout << "Waiting for a response..." << std::endl;
    if(socket.receive(packet) != fr::Socket::Success)
    {
        std::cout << "Failed to receive server response!" << std::endl;
        return -2;
    }

    //Extract the response, you can surround this in a try/catch block to catch errors
    std::string str1, str2;
    float age;
    packet >> str1 >> age >> str2;
    std::cout << "Server sent: " << str1 << ", " << age << ", " << str2 << "\n\n\n" << std::endl;
    return 0;
}


int main()
{

    //Try and connect to the server, create a new TCP object for the job and then connect
    fr::TcpSocket socket;  //Use an fr::SSLSocket if SSL
    if(socket.connect(SERVER_IP, SERVER_PORT, std::chrono::seconds(20)) != fr::Socket::Success)
    {
        //Failed to connect
        std::cout << "Failed to connect to: " << SERVER_IP << ":" << SERVER_PORT << std::endl;
        return -1;
    }

    //For storing user input and send_a_packet response
    std::string op_str;
    int rtn = 0;

    //Keep going until either we, or the server closes the connection
    while(true)
    {
        //Ask the user what to do
        std::cout << "Choose what you want to do, `c` for `continue`, `q` for `quit`:" << std::endl;
        std::cin >> op_str;
        if(op_str.length() > 1)
        {
            std::cout << "Seems that you inputted more than one character, please retry." << std::endl;
            continue;
        }

        switch(op_str[0])
        {
            case 'c':
                std::cout << "continue" << std::endl;
                rtn = send_a_packet(socket);
                break;
            case 'q':
                break;
            default:
            std::cout << "Invalid input!" << std::endl;
        }

        //Exit/error check
        if(op_str[0] == 'q')
            break;
        if(rtn != 0)
            break;
    }
    
    std::cout << "All done, bye!" << std::endl;
}

