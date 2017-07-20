//
// Created by fred.nicolson on 30/05/17.
//

#include <gtest/gtest.h>
#include <frnetlib/URL.h>

TEST(URLTest, full_parse)
{
    fr::URL url("http://example.com:80/path/path?query=10&bob=20#frag");
    ASSERT_EQ(url.get_host(), "example.com");
    ASSERT_EQ(url.get_scheme(), fr::URL::HTTP);
    ASSERT_EQ(url.get_path(), "path/path");
    ASSERT_EQ(url.get_query(), "query=10&bob=20");
    ASSERT_EQ(url.get_fragment(), "frag");
}

TEST(URLTest, port_guess)
{
    fr::URL url("https://example.com");
    ASSERT_EQ(url.get_port(), "443");
}

TEST(URLTest, partial_parse)
{
    fr::URL url("example.com/?query=10#frag");
    ASSERT_EQ(url.get_host(), "example.com");
    ASSERT_EQ(url.get_query(), "query=10");
    ASSERT_EQ(url.get_fragment(), "frag");
}

TEST(URLTest, fragment_test)
{
    fr::URL url("example.com/#frag");
    ASSERT_EQ(url.get_fragment(), "frag");
}

TEST(URLTest, path_test)
{
    fr::URL url("example.com/path/hey#frag");
    ASSERT_EQ(url.get_path(), "path/hey");
    ASSERT_EQ(url.get_fragment(), "frag");
}