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
#include "SSLContext.h"
#include "SSLListener.h"

int main()
{
    std::shared_ptr<fr::SSLContext> ssl_context(new fr::SSLContext("certs.crt"));

    fr::HttpSocket<fr::SSLSocket> socket(ssl_context);
    std::string addr;
    std::cin >> addr;
    socket.connect(addr, "443");

    fr::HttpRequest request;
    socket.send(request);

    fr::HttpResponse response;
    socket.receive(response);

    std::cout << response.get_body() << std::endl;
    return 0;
}