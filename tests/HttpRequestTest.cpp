#include "gtest/gtest.h"
#include <frnetlib/HttpRequest.h>

TEST(HttpRequestTest, get_request_parse)
{
    //The test request to parse
    const std::string raw_request =
            "GET /index.html HTTP/1.1\n"
            "Host: frednicolson.co.uk\r\n"
            "Content-Type: application/x-www-form-urlencoded\r\n"
            "My-Header:      header1\n"
            "My-Other-Header:header2\r\n"
            "Cache-Control: no-cache\r\n\r\n";

    //Parse it
    fr::HttpRequest request;
    ASSERT_EQ(request.parse(raw_request.c_str(), raw_request.size()), false);

    //Check that the request type is intact
    ASSERT_EQ(request.get_type(), fr::Http::Get);

    //Test that URI is intact
    ASSERT_EQ(request.get_uri(), "/index.html");

    //Test that headers exist
    ASSERT_EQ(request.header_exists("host"), true);
    ASSERT_EQ(request.header_exists("contEnt-type"), true);
    ASSERT_EQ(request.header_exists("my-HeadEr"), true);
    ASSERT_EQ(request.header_exists("my-other-header"), true);
    ASSERT_EQ(request.header_exists("cache-control"), true);
    ASSERT_EQ(request.header_exists("non-existant"), false);

    //Check that headers are intact
    ASSERT_EQ(request.header("host"), "frednicolson.co.uk");
    ASSERT_EQ(request.header("content-type"), "application/x-www-form-urlencoded");
    ASSERT_EQ(request.header("My-Other-Header"), "header2");
    ASSERT_EQ(request.header("My-Header"), "header1");
}

TEST(HttpRequestTest, post_request_parse)
{
    const std::string raw_request =
            "POST /index.html HTTP/1.1\r\n"
            "\r\n"
            "post_data=data1&some_more_post_data=data23\r\n\r\n";

    //Parse it
    fr::HttpRequest request;
    ASSERT_EQ(request.parse(raw_request.c_str(), raw_request.size()), false);

    //Check that the request type is intact
    ASSERT_EQ(request.get_type(), fr::Http::Post);

    //Test that URI is intact
    ASSERT_EQ(request.get_uri(), "/index.html");

    //Parse code is the same for GET, so skip header checks. Test if POST data exists.
    ASSERT_EQ(request.post_exists("post_data"), true);
    ASSERT_EQ(request.post_exists("some_mOre_posT_data"), true);
    ASSERT_EQ(request.post_exists("non_existant"), false);

    //Check that the POST data is valid
    ASSERT_EQ(request.post("post_dAta"), "data1");
    ASSERT_EQ(request.post("some_more_post_data"), "data23");
}

TEST(HttpRequestTest, request_type_parse)
{
    const std::string get_request = "GET / HTTP/1.1\r\n\r\n";
    const std::string post_request = "POST / HTTP/1.1\r\n\r\n";
    const std::string put_request = "PUT / HTTP/1.1\r\n\r\n";
    const std::string delete_request = "DELETE / HTTP/1.1\r\n\r\n";
    const std::string patch_request = "PATCH / HTTP/1.1\r\n\r\n";
    const std::string invalid_request = "INVALID / HTTP/1.1\r\n\r\n";

    fr::HttpRequest request;
    request.parse(get_request.c_str(), get_request.size());
    ASSERT_EQ(request.get_type(), fr::Http::Get);
    request = {};

    request.parse(post_request.c_str(), post_request.size());
    ASSERT_EQ(request.get_type(), fr::Http::Post);
    request = {};

    request.parse(put_request.c_str(), put_request.size());
    ASSERT_EQ(request.get_type(), fr::Http::Put);
    request = {};

    request.parse(delete_request.c_str(), delete_request.size());
    ASSERT_EQ(request.get_type(), fr::Http::Delete);
    request = {};

    request.parse(patch_request.c_str(), patch_request.size());
    ASSERT_EQ(request.get_type(), fr::Http::Patch);
    request = {};

    request.parse(invalid_request.c_str(), invalid_request.size());
    ASSERT_EQ(request.get_type(), fr::Http::Unknown);
    request = {};
}

TEST(HttpRequestTest, request_construction)
{
    //todo: more tests
}