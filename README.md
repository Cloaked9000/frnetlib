# frnetlib 
![Build Status](https://travis-ci.org/Cloaked9000/frnetlib.svg?branch=master)

Frnetlib, is a cross-platform, small and fast networking library written in C++. There are no library dependencies (unless you want to use SSL, in which case MbedTLS is required), and it should compile fine with any C++11 compliant compiler. The API should be considered relatively stable, but things could change as new features are added, or existing ones changed.

Frnetlib is tested on both Linux and Windows and currently supports:
* HTTP/HTTPS clients
* HTTP/HTTPS servers
* WebSocket clients
* WebSocket servers
* Raw socket communication
* Framed messaging of inbuilt and custom types

# Connecting to a Socket:

```c++
#include <TcpSocket.h>

fr::TcpSocket socket;
if(socket.connect("127.0.0.1", "8081", std::chrono::seconds(10)) != fr::Socket::Status::Success)
{
    //Failed to connect
}
```
Here, we create a new fr::TcpSocket and connect it to an address. Simple. fr::TcpSocket is the core class, used to send and receive data over TCP, either with frnetlib's own message framing, or raw data for communicating with other protocols. Unfortunately, UDP is not supported at this point. Sockets are blocking by default.

# Listening and accepting connections:

```c++
#include <TcpSocket.h>
#include <TcpListener.h>

fr::TcpListener listener;

//Bind to a port
if(listener.listen("8081") != fr::Socket::Status::Success)
{
    //Failed to bind to port
}

//Accept a connection
fr::TcpSocket client;
if(listener.accept(client) != fr::Socket::Status::Success)
{
    //Failed to accept a new connection
}

```
Here we create a new fr::TcpListener, which is used to listen for incoming connections and accept them. Calling fr::TcpListener::listen(port) will bind the listener to a port, allowing you to receive connections on that port. Next a new fr::TcpSocket is created, which is where the accepted connection is stored, to send data through the new connection, we do so though 'client' from now on. fr::TcpListener's can accept as many new connections as you want. You don't need a new one for each client. 

# Using SSL

```c++
#include <SSLSocket.h>
#include <SSLContext.h>
#include <SSLListener.h>

std::shared_ptr<fr::SSLContext> ssl_context(new fr::SSLContext("certs.crt")); //Creates a new 'SSL' context. This stores certificates and is shared between SSL enabled objects.
ssl_conext->load_ca_certs_from_file(filepath); //This, or 'load_ca_certs_from_memory' should be called on the context, to load your SSL certificates.

fr::SSLListener listener(ssl_context, "crt_path", "pem_path", "private_key_path"); //This is the SSL equivilent to fr::TcpListener

fr::SSLSocket socket(ssl_context); //This is the SSL equivilent to fr::TcpSocket

```
As you've probably noticed, everything unencrypted has it's equivalent encrypted counterpart, usually just by replacing 'TCP' with 'SSL' and providing an SSLContext object.
fr::SSLContext stores SSL information which need not be duplicated across each socket and listener, such as the random number generator, and public key list. It is *important* to build mbedtls with thread protection enabled, if your program is multithreaded. This SSLContext object can then be passed to any SSL sockets or listeners which you may create.

SSLListener accepts a lot more arguments than its unencrypted counterpart, TcpListener, and it needs the filepaths to your SSL certificates and keys to properly authenticate with clients. 

# Sending packets:

```c++
#include <Packet.h>

fr::Packet packet;
packet << "Hello there, I am" << (float)1.2 << "years old";

if(socket.send(packet) != fr::Socket::Status::Success)
{
    //Failed to send packet
}
```
To send messages using frnetlib's framing, use fr::Packet. Data added to the packet, using the '<<' operator will automatically be packed and converted to network byte order if applicable. The data should be unpacked using the '>>' operator in the same order as it was packed. It is *important* to explicitly typecast non-strings as shown above, otherwise the compiler might interpret '10' as a uint16_t instead of a uint32_t like you might have wanted, scrambling the data. To send the packet, just call fr::TcpSocket::send.

# Receiving packets:

```c++
fr::Packet packet;
if(client.receive(packet) != fr::Socket::Status::Success)
{
    //Failed to receive packet
}

std::string str1, str2;
float age;
packet >> str1 >> age >> str2;
```
Effectively the reverse of sending packets. We call fr::TcpSocket::receive, passing it a fr::Packet object, to receive a packet, and then extract the data in the same order that we packed it. fr::Socket::receive is blocking by default, but you can toggle this using fr::Socket::set_blocking(). If the socket is blocking when you call receive, it will wait until data has been received before returning. If the socket is non-blocking, then it will return immediately, even if the socket is not ready to receive data. If the socket is non-blocking and is not ready to receive data when you call receive, then it will return a fr::Socket::Status::WouldBlock value.

# A simple HTTP server:

```c++
#include <HttpRequest.h>
#include <HttpResponse.h>

fr::TcpSocket client;                 //fr::TcpSocket for HTTP. fr::SSLSocket for HTTPS.
fr::TcpListener listener;             //Use an fr::SSLListener if HTTPS.

//Bind to a port
if(listener.listen("8081") != fr::Socket::Status::Success)
{
    //Failed to bind to port
}

while(true)
{
    //Accept a new connection
    if(listener.accept(client) != fr::Socket::Status::Success)
    {
        //Failed to accept client
    }

    //Receive client HTTP request
    fr::HttpRequest request;
    if(client.receive(request) != fr::Socket::Status::Success)
    {
        //Failed to receive request
    }

    //Construct a response
    fr::HttpResponse response;
    response.set_body("<h1>Hello, World!</h1>");

    //Send it
    if(client.send(response) != fr::Socket::Status::Success)
    {
        //Failed to send response;
    }

    //Close connection
    client.close_socket();
}
```
After binding to the port, we infinitely try and receive a new request, construct a response with the body of 'Hello, World!' and send it back to the client before closing the socket. fr::HttpRequest, and fr::HttpResponse both inherit fr::Sendable, which allows them to be sent and received through sockets just like fr::Packets.

fr::HttpRequest objects are used for dealing with data being sent *to* the server, whereas fr::HttpResponse objects are used for dealing with data being sent *from* the server. GET/POST/Header information can be manipulated the same as in the example below.

# A simple HTTP client: 

```c++
#include <HttpRequest.h>
#include <HttpResponse.h>

//Connect to the website example.com on port 80, with a 10 second connection timeout
fr::TcpSocket socket;
if(socket.connect("example.com", "80", std::chrono::seconds(10)) != fr::Socket::Status::Success)
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
if(socket.send(request) != fr::Socket::Status::Success)
{
    //Failed to send request
}

//Wait for a response
fr::HttpResponse response;
if(socket.receive(response) != fr::Socket::Status::Success)
{
    //Failed to receive response
}

//Print out the response
std::cout << request.get_body() << std::endl;
```
Here we create an fr::TcpSocket object, connect to a domain (don't include the 'http://' bit). The socket is non-SSL and so the underlying socket type is fr::TcpSocket. If this were an SSL socket, then it'd be fr::SSLSocket. After connecting, we construct a fr::HttpRequest object to send to the server, adding in some GET arguments, POST arguments and a request header.

You can both set and get GET/POST data through the fr::(HttpRequest/HttpResponse)::(get/post) functions. And access/set headers though the [] operator. Once we've sent a request, we wait for a response. Once received, we print out the body of the response and exit.

# Sending custom objects:
```c++
class MyClass : public fr::Packetable
{
    int member_a, member_b;

    virtual void pack(fr::Packet &o) const override
    {
        o << member_a << member_b;
    }

    virtual void unpack(fr::Packet &o) override
    {
        o >> member_a >> member_b;
    }
};
```
You can add support for adding your own classes to fr::Packets, by inheriting fr::Packetable and implementing the 'pack' and 'unpack' functions. Your 'pack' function should add your class members to the provided packet object, and your 'unpack' function should take them back out. This allows for code like this:

```c++
MyClass my_class;
fr::Packet packet;
packet << my_class;
packet >> my_class;
```
