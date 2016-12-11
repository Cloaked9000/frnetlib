# frnetlib

Frnetlib, is a small and fast networking library written in C++. It can be used for both messaging and for sending/receiving HTTP requests. There are no library dependencies, and it should compile fine with any C++11 complient compiler. The API should be considered relatively stable, but things could change as new features are added, given that the library is still in the early stages of deveopment.

# Connecting to a Socket:

```c++
#include <TcpSocket.h>

fr::TcpSocket socket;
if(socket.connect("127.0.0.1", "8081") != fr::Socket::Success)
{
    //Failed to connect
}
```
Here, we create a new fr::TcpSocket and connect it to an address. Simple. fr::TcpSocket is the core class, used to send and receive data over TCP, either with frnetlib's own message framing, or raw data for communicating with other protocols. Unfortunately, UDP is not supported at this point. Sockets are blocking by default, and there is currently no way of disabling blocking.

# Listening and accepting connections:

```c++
#include <TcpSocket.h>
#include <TcpListener.h>

fr::TcpListener listener;

//Bind to a port
if(listener.listen("8081") != fr::Socket::Success)
{
    //Failed to bind to port
}

//Accept a connection
fr::TcpSocket client;
if(listener.accept(client) != fr::Socket::Success)
{
    //Failed to accept a new connection
}

```
Here we create a new fr::TcpListener, which is used to listen for incomming connections and accept them. Calling fr::TcpListener::listen(port) will bind the listener to a port, allowing you to receive connections on that port. Next a new fr::TcpSocket is created, which is where the accepted connection is stored, to send data through the new connection, we do so though 'client' from now on. fr::TcpListener's can accept as many new connections as you want. You don't need a new one for each client. 

# Sending packets:

```c++
#include <Packet.h>

fr::Packet packet;
packet << "Hello there, I am" << (float)1.2 << "years old";

if(socket.send(packet) != fr::Socket::Success)
{
    //Failed to send packet
}
```
To send messages using frnetlib's framing, use fr::Packet. Data added to the packet, using the '<<' operator will automatically be packed and converted to network byte order if applicable. The data should be unpacked using the '>>' operator in the same order as it was packed. It is *important* to explicitly typecast non-strings as shown above, otherwise the compiler might interpret '10' as a uint16_t instead of a uint32_t like you might have wanted, scrambling the data. To send the packet, just call fr::TcpSocket::send.  

# Receiving packets:

```c++
fr::Packet packet;
if(client.receive(packet) != fr::Socket::Success)
{
    //Failed to receive packet
}

std::string str1, str2;
float age;
packet >> str1 >> age >> str2;
```
Effectively the reverse of sending packets. We call fr::TcpSocket::receive, passing it a fr::Packet object, to receive a packet, and then extract the data in the same order that we packed it.

# A simple HTTP server:

```c++
#include <HttpRequest.h>
#include <HttpResponse.h>

fr::HttpSocket client;
fr::TcpListener listener;

//Bind to a port
if(listener.listen("8081") != fr::Socket::Success)
{
    //Failed to bind to port
}

while(true)
{
    //Accept a new connection
    if(listener.accept(client) != fr::Socket::Success)
    {
        //Failed to accept client
    }

    //Receive client HTTP request
    fr::HttpRequest request;
    if(client.receive(request) != fr::Socket::Success)
    {
        //Failed to receive request
    }

    //Construct a response
    fr::HttpResponse response;
    response.set_body("<h1>Hello, World!</h1>");

    //Send it
    if(client.send(response) != fr::Socket::Success)
    {
        //Failed to send response;
    }

    //Close connection
    client.close();
}
```
To deal with HTTP requests, we need to use fr::HttpSockets, to automatically construct and extract fr::HttpResponse/fr::HttpRequest objects. fr::HttpSocket is internally just a wrapper around fr::TcpSocket for HTTP requests, allowing you to add it to a fr::TcpListener the same you would a fr::TcpSocket. After binding to the port, we infinitely try and receive a new request, construct a response with the body of 'Hello, World!' and send it back to the client before closing the socket. 

fr::HttpRequest objects are used for dealing with data being sent *to* the server, whereas fr::HttpResponse objects are used for dealing with data being sent *from* the server. GET/POST/Header information can be manipulated the same as in the example below.

# A simple HTTP client: 

```c++
#include <HttpSocket.h>
#include <HttpRequest.h>
#include <HttpResponse.h>

//Connect to the website
fr::HttpSocket socket;
if(socket.connect("example.com", "80") != fr::Socket::Success)
{
    //Failed to connect to site
}

//Construct a request with some data
fr::HttpRequest request;
request.get("name") = "bob";
request.get("age") = 10;
request.post("isalive") = "true";
request["my-header"] = "value";

//Send the request
if(socket.send(request) != fr::Socket::Success)
{
    //Failed to send request
}

//Wait for a response
fr::HttpResponse response;
if(socket.receive(response) != fr::Socket::Success)
{
    //Failed to receive response
}

//Print out the response
std::cout << request.get_body() << std::endl;
```
Here we create a fr::HttpSocket object, connect to a domain (don't include the 'http://' bit). SSL support is planned but not implemented just yet. After connecting, we construct a fr::HttpRequest object to send to the server, adding in some GET arguments, POST arguments and a request header. 

You can both set and get GET/POST data through the fr::(HttpRequest/HttpResponse)::(get/post) functions. And access/set headers though the [] operator. Once we've sent a request, we wait for a response. Once received, we print out the body of the response and exit.

# Blocking on multiple sockets simultaneously:

```c++
#include <SocketSelector.h>

//Bind to port
fr::TcpListener listener;
if(listener.listen("8081") != fr::Socket::Success)
{
    std::cout << "Failed to listen to port" << std::endl;
    return;
}

//Create a selector and a container for holding connected clients
fr::SocketSelector selector;
std::vector<std::unique_ptr<fr::TcpSocket>> clients;

//Add our connection listener to the selector
selector.add(listener);

//Infinitely loop, waiting for connections or data
while(selector.wait())
{
    //If the listener is ready, that means we've got a new connection
    if(selector.is_ready(listener))
    {
        //Accept the new connection
        std::unique_ptr<fr::TcpSocket> new_client = std::unique_ptr<fr::TcpSocket>(new fr::TcpSocket());
        if(listener.accept(*new_client) == fr::Socket::Success)
        {
            //Add them to the selector, and our socket list
            selector.add(*new_client);
            clients.emplace_back(std::move(new_client));
        }
    }
    else
    {
        //Iterate over our clients to find which one has sent data
        for(auto iter = clients.begin(); iter != clients.end();)
        {
            fr::TcpSocket &socket = **iter;
            if(selector.is_ready(socket))
            {
                fr::Packet packet;
                if(socket.receive(packet) == fr::Socket::Success)
                {
                    //Do something with packet
                    //...
                    
                    iter++;
                }
                else
                {
                    //Client has disconnected. Remove them.
                    selector.remove(socket);
                    iter = clients.erase(iter);
                }
            }
            else
            {
                iter++;
            }
        }
    }
}
```
fr::SocketSelector can be used to monitor lots of blocking sockets at once (both fr::TcpSocket's and fr::HttpSocket's), without polling, to see when data is being received or a connection has closed. To add a socket, just call fr::SocketSelector::add, and to remove a socket, which must be done before the socket object is destroyed, call fr::SocketSelector::remove. You can add as many fr::Socket's as you want.It is also important to add your fr::TcpListener to the selector, otherwise you wont be able to accept new connections whilst blocking.

Once added, you can call fr::SocketSelector::wait() to wait for socket events. You can not currently add a wait timeout, though this is planned and should be added soon. If any of the sockets added to the selector send data or disconnect, then it will return.

To find which socket actually did something, we need to call fr::SocketSelector::is_ready(), passing it the socket to check. It'll return true if it was this socket that sent data, false otherwise. First we check our selector, and accept a new connection if it was that. Otherwise, we check through all of the connected clients. If the fr::Socket::receive() call failed on the socket, then it must have disconnected, so we remove it from the client list, and remove it from the selector.
