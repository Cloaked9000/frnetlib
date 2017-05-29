#include <iostream>
#include "frnetlib/Packet.h"


int main()
{
    fr::Packet packet;
    std::vector<std::pair<int, int>> bob = {{1, 2}, {3, 4}};
    packet << bob;
    bob.clear();

    packet >> bob;
    std::cout << bob[0].first << ", " << bob[0].second << ", " << bob[1].first << ", " << bob[1].second << std::endl;
}