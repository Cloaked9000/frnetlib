#include <iostream>
#include <SSLListener.h>
#include "include/Packet.h"
#include "include/TcpSocket.h"
#include "include/TcpListener.h"
#include "include/SocketSelector.h"
#include "HttpSocket.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "SSLSocket.h"

int main()
{
    //Bind to port
    fr::SSLListener listener("key.crt", "key.pem", "private.key");
    if(listener.listen("8080") != fr::Socket::Success)
    {
        //Error
    }

    //Create socket selector and add listener
    fr::SocketSelector selector;
    selector.add(listener);

    //Create vector to store open connections
    std::vector<std::unique_ptr<fr::Socket>> connections;

    //Infinitely loop. No timeout is specified so it will not return false.
    while(selector.wait())
    {
        //Check if it was the selector who sent data
        if(selector.is_ready(listener))
        {
            std::unique_ptr<fr::HttpSocket<fr::SSLSocket>> socket(new fr::HttpSocket<fr::SSLSocket>);
            if(listener.accept(*socket) == fr::Socket::Success)
            {
                selector.add(*socket);
                connections.emplace_back(std::move(socket));
            }
        }

        //Else it must have been one of the clients
        else
        {
            //Find which client send the data
            for(auto iter = connections.begin(); iter != connections.end();)
            {
                //Eww
                fr::HttpSocket<fr::SSLSocket> &client = (fr::HttpSocket<fr::SSLSocket>&)**iter;

                //Check if it's this client
                if(selector.is_ready(client))
                {
                    //It is, so receive their HTTP request
                    fr::HttpRequest request;
                    if(client.receive(request) == fr::Socket::Success)
                    {
                        //Send back a HTTP response containing 'Hello, World!'
                        fr::HttpResponse response;
                        response.set_body("<h1>frnetlib test page</h1>");
                        client.send(response);

                        //Remove them from the selector and close the connection
                        selector.remove(client);
                        client.close();
                        iter = connections.erase(iter);
                    }
                    else
                    {
                        iter++;
                    }
                }
                else
                {
                    iter++;
                }
            }
        }
    }


    return 0;
}