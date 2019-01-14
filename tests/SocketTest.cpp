//
// Created by fred.nicolson on 25/09/17.
//

#include <gtest/gtest.h>
#include <frnetlib/Socket.h>

TEST(SocketTest, status_to_string_valid)
{
    ASSERT_EQ(fr::Socket::status_to_string(fr::Socket::Status::Unknown), "Unknown");
    ASSERT_EQ(fr::Socket::status_to_string(fr::Socket::Status::HttpBodyTooBig), "HTTP Body Too Big");
}

TEST(SocketTest, status_to_string_invalid)
{

    auto str = fr::Socket::status_to_string(static_cast<fr::Socket::Status>(-1));
    ASSERT_EQ(str, "Unknown");
    str = fr::Socket::status_to_string(static_cast<fr::Socket::Status>(99999));
    ASSERT_EQ(str, "Unknown");
}