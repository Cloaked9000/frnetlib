#include <iostream>
#include <Packet.h>

int main()
{
    Packet packet;
    packet << (uint16_t)15000 << (uint16_t)200 << (uint32_t)9299221 << (uint64_t)9223372036854775807 << (float)1.22 << (double)192.212;
    std::cout << packet.construct_packet() << std::endl;

    uint16_t result, result2;
    uint32_t result3;
    uint64_t result4;
    float result5;
    double result6;
    packet >> result >> result2 >> result3 >> result4 >> result5 >> result6;

    std::cout << result << ", " << result2 << ", " << result3 << ", " << result4 << ", " << result5 << ", " << result6 << std::endl;
    return 0;
}