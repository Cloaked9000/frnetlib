#include <frnetlib/TcpSocket.h>
#include <frnetlib/TcpListener.h>
#define PORT "8081"


int main()
{
    //Create a new TCP socket to maintain connections, and a new Tcp Listener to accept connections
    fr::TcpSocket client;      //fr::SSLSocket for SSL
    fr::TcpListener listener;  //fr::SSLListener for HTTPS

    //Bind to a port
    if(listener.listen(PORT) != fr::Socket::Success)
    {
        std::cout << "Failed to bind to port: " << PORT << std::endl;
        return -1;
    }

    std::cout << "Listener is listening on port: " << PORT << "..." << std::endl;

    //Start accepting connections
    while(true)
    {
        std::cout << "Waiting for a new connection ..." << std::endl;

        //Accept a new connection
        if(listener.accept(client) != fr::Socket::Success)
        {
            std::cout << "Failed to accept client, shutdown" << std::endl;
            break;
        }

        while(true) //Infinite loop to keep the connection active
        {
            try
            {
                //Receive an fr::Packet from the client
                fr::Packet packet;
                if(client.receive(packet) != fr::Socket::Success)
                {
                    std::cout << "Failed to receive request" << std::endl;
                    client.close_socket();
                    break;
                }

                //Extract the data from the packet
                std::string str1, str2;
                float age;
                packet >> str1 >> age >> str2;

                //Print out what we got
                std::cout << "Client sent:" << str1 << ", " << age << ", " << str2 << std::endl;

                //Send back the same packet
                if(client.send(packet) != fr::Socket::Success)
                {
                    throw std::string("Failed to send packet to client");
                }
            }
            catch(const std::exception &e)
            {
                //Print out what happened
                std::cout << "Error: " << e.what() << std::endl;

                //Close connection
                client.close_socket();
                break;
            }
        }


    }

    return 0;
}

