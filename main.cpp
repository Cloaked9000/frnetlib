#include <iostream>
#include <frnetlib/SSLListener.h>
#include "frnetlib/Packet.h"
#include "frnetlib/TcpSocket.h"
#include "frnetlib/TcpListener.h"
#include "frnetlib/SocketSelector.h"
#include "frnetlib/HttpSocket.h"
#include "frnetlib/HttpRequest.h"
#include "frnetlib/HttpResponse.h"
#include "frnetlib/SSLSocket.h"
#include "frnetlib/SSLContext.h"
#include "frnetlib/SSLListener.h"

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