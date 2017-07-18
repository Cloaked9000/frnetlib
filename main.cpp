#include <iostream>
#include <frnetlib/SSLListener.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include "frnetlib/Packet.h"
#include "frnetlib/TcpSocket.h"
#include "frnetlib/TcpListener.h"
#include "frnetlib/SocketSelector.h"
#include "frnetlib/HttpRequest.h"
#include "frnetlib/HttpResponse.h"
#include "frnetlib/SSLSocket.h"
#include "frnetlib/SSLContext.h"
#include "frnetlib/SSLListener.h"

enum Enum : uint32_t
{
    E1 = 0,
    E2 = 1,
    E3 = 2,
};
int main()
{
    fr::Packet packet;
    std::vector<std::pair<int, int>> bob = {{1, 2}, {3, 4}};
    packet << bob;
    bob.clear();

    packet >> bob;
    std::cout << bob[0].first << ", " << bob[0].second << ", " << bob[1].first << ", " << bob[1].second << std::endl;
}