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
    ASSERT_EQ(url.get_path(), "/path/path");
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
    ASSERT_EQ(url.get_path(), "/path/hey");
    ASSERT_EQ(url.get_fragment(), "frag");
}

TEST(URLTest, uri_test)
{
    fr::URL url("http://example.com:80/path/path?query=10#frag");
    ASSERT_EQ(url.get_uri(), "/path/path?query=10#frag");
}

TEST(URLTest, uri_test2)
{
    fr::URL url("http://example.com:80/path/path?query=10");
    ASSERT_EQ(url.get_uri(), "/path/path?query=10");
}

TEST(URLTest, uri_test3)
{
    fr::URL url("http://example.com:80/path/path#frag");
    ASSERT_EQ(url.get_uri(), "/path/path#frag");
}

TEST(URLTest, uri_test4)
{
    fr::URL url("http://example.com:80/?bob=10#frag");
    ASSERT_EQ(url.get_uri(), "/?bob=10#frag");
}

TEST(URLTest, schema_parse_test)
{
    fr::URL url("127.0.0.1:2020");
    fr::URL url2;
    url2.parse("127.0.0.1:2020");
    ASSERT_TRUE(url == url2);
}

TEST(URLTest, get_url_test)
{
    ASSERT_EQ(fr::URL("127.0.0.1:2020").get_url(), "127.0.0.1:2020");
    ASSERT_EQ(fr::URL("https://127.0.0.1:2020").get_url(), "https://127.0.0.1:2020");
    ASSERT_EQ(fr::URL("https://127.0.0.1").get_url(), "https://127.0.0.1:443");
    ASSERT_EQ(fr::URL("127.0.0.1/hello.php?x=10").get_url(), "127.0.0.1/hello.php?x=10");
    ASSERT_EQ(fr::URL("/hello.php").get_url(), "/hello.php");
}

TEST(URLTest, test_modify_url)
{
    fr::URL url("https://example.com:2020");
    url.set_host("example.co.uk");
    ASSERT_EQ(url.get_url(), "https://example.co.uk:2020");
}