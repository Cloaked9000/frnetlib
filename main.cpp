#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include "frnetlib/Packet.h"
#include "frnetlib/TcpSocket.h"
#include "frnetlib/TcpListener.h"
#include "frnetlib/SocketSelector.h"
#include "frnetlib/HttpRequest.h"
#include "frnetlib/HttpResponse.h"

enum Enum : uint32_t
{
    E1 = 0,
    E2 = 1,
    E3 = 2,
};

size_t get_vector_size(const std::vector<int> &vec)
{
    return vec.size();
}

int main()
{
    std::vector<int> source{1, 2, 3};
    std::cout << get_vector_size(source) << std::endl;
}