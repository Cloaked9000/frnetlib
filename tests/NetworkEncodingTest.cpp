//
// Created by fred.nicolson on 01/02/18.
//

#include <gtest/gtest.h>
#include "frnetlib/NetworkEncoding.h"

constexpr bool is_little_endian()
{
    unsigned short x=0x0001;
    auto p = reinterpret_cast<unsigned char*>(&x);
    return *p != 0;
}

TEST(NetworkEncodingTest, test_htonf)
{
    float input = std::numeric_limits<float>::max();
    float result = htonf(input);

    if(is_little_endian())
    {
        float manual;
        std::reverse_copy((char*)&input, (char*)&input + sizeof(input), (uint8_t*)&manual);
        ASSERT_EQ(memcmp(&result, &manual, sizeof(manual)), 0);
    }
    else
    {
        ASSERT_EQ(result, input);
    }
}

TEST(NetworkEncodingTest, test_ntohf)
{
    float input = std::numeric_limits<float>::max();
    float encoded = htonf(input);
    float decoded = ntohf(encoded);
    ASSERT_EQ(input, decoded);
}

TEST(NetworkEncodingTest, test_htond)
{
    double input = std::numeric_limits<double>::max();
    double result = htond(input);

    if(is_little_endian())
    {
        double manual;
        std::reverse_copy((char*)&input, (char*)&input + sizeof(input), (uint8_t*)&manual);
        ASSERT_EQ(memcmp(&result, &manual, sizeof(manual)), 0);
    }
    else
    {
        ASSERT_EQ(result, input);
    }
}

TEST(NetworkEncodingTest, test_ntohd)
{
    double input = std::numeric_limits<double>::max();
    double encoded = htond(input);
    double decoded = ntohd(encoded);
    ASSERT_EQ(input, decoded);
}

TEST(NetworkEncodingTest, test_htonll)
{
    uint64_t input = std::numeric_limits<uint64_t>::max();
    uint64_t result = htonll(input);

    if(is_little_endian())
    {
        uint64_t manual;
        std::reverse_copy((char*)&input, (char*)&input + sizeof(input), (uint8_t*)&manual);
        ASSERT_EQ(manual, result);
    }
    else
    {
        ASSERT_EQ(result, input);
    }
}

TEST(NetworkEncodingTest, test_ntohll)
{
    uint64_t input = std::numeric_limits<uint64_t>::max();
    uint64_t encoded = htonll(input);
    uint64_t decoded = ntohll(encoded);
    ASSERT_EQ(input, decoded);
}