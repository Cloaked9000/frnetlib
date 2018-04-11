//
// Created by fred.nicolson on 01/02/18.
//

#include <gtest/gtest.h>
#include <limits>
#include <algorithm>
#include "frnetlib/NetworkEncoding.h"

constexpr bool is_little_endian()
{
    unsigned short x=0x0001;
    auto p = reinterpret_cast<unsigned char*>(&x);
    return *p != 0;
}

TEST(NetworkEncodingTest, test_htonf)
{
    float input = std::numeric_limits<float>::max() - 50;
    float result = fr::htonf(input);

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
    float input = std::numeric_limits<float>::max() - 50;
    float encoded = fr::htonf(input);
    float decoded = fr::ntohf(encoded);
    ASSERT_EQ(input, decoded);
}

TEST(NetworkEncodingTest, test_htond)
{
    double input = std::numeric_limits<double>::max() - 50;
    double result = fr::htond(input);

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
    double encoded = fr::htond(input);
    double decoded = fr::ntohd(encoded);
    ASSERT_EQ(input, decoded);
}

TEST(NetworkEncodingTest, test_htonll)
{
    uint64_t input = std::numeric_limits<uint64_t>::max() - 50;
    uint64_t result = fr_htonll(input);

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
    uint64_t input = std::numeric_limits<uint64_t>::max() - 50;
    uint64_t encoded = fr_htonll(input);
    uint64_t decoded = fr_ntohll(encoded);
    ASSERT_EQ(input, decoded);
}