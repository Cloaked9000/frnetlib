//
// Created by fred.nicolson on 25/09/17.
//

#include <gtest/gtest.h>
#include <frnetlib/Socket.h>

TEST(SocketTest, status_to_string_valid)
{
    ASSERT_EQ(fr::Socket::status_to_string(fr::Socket::Status::Unknown), "Unknown");
    ASSERT_EQ(fr::Socket::status_to_string(fr::Socket::Status::HttpBodyTooBig), "HTTP body too big");
}

TEST(SocketTest, status_to_string_invalid)
{
    try
    {
        auto str = fr::Socket::status_to_string(static_cast<fr::Socket::Status>(-1));
    }
    catch(const std::logic_error &)
    {
        try
        {
            auto str = fr::Socket::status_to_string(static_cast<fr::Socket::Status>(99999));
        }
        catch(const std::logic_error &)
        {
            return;
        }
    }
    ASSERT_TRUE(false);
}