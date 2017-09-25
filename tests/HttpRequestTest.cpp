#include "gtest/gtest.h"
#include <frnetlib/HttpRequest.h>

TEST(HttpRequestTest, get_request_parse)
{
    //The test request to parse
    const std::string raw_request =
            "GET /index.html?var=bob&other=trob HTTP/1.1\n"
            "Host: frednicolson.co.uk\r\n"
            "Content-Type: application/x-www-form-urlencoded\r\n"
            "My-Header:      header1\n"
            "My-Other-Header:header2\r\n"
            "Cache-Control: no-cache\r\n\r\n";

    //Parse it
    fr::HttpRequest request;
    ASSERT_EQ(request.parse(raw_request.c_str(), raw_request.size()), fr::Socket::Success);

    //Check that the request type is intact
    ASSERT_EQ(request.get_type(), fr::Http::Get);

    //Test that URI is intact
    ASSERT_EQ(request.get_uri(), "/index.html");

    //Test that headers exist
    ASSERT_EQ(request.header_exists("Host"), true);
    ASSERT_EQ(request.header_exists("Content-Type"), true);
    ASSERT_EQ(request.header_exists("My-Header"), true);
    ASSERT_EQ(request.header_exists("My-Other-Header"), true);
    ASSERT_EQ(request.header_exists("Cache-Control"), true);
    ASSERT_EQ(request.header_exists("non-existant"), false);

    //Check that headers are intact
    ASSERT_EQ(request.header("Host"), "frednicolson.co.uk");
    ASSERT_EQ(request.header("Content-Type"), "application/x-www-form-urlencoded");
    ASSERT_EQ(request.header("My-Other-Header"), "header2");
    ASSERT_EQ(request.header("My-Header"), "header1");

    //Test that GET variables exist
    ASSERT_EQ(request.get_exists("var"), true);
    ASSERT_EQ(request.get_exists("other"), true);
    ASSERT_EQ(request.get_exists("fake"), false);

    //Ensure that GET variables are valid
    ASSERT_EQ(request.get("var"), "bob");
    ASSERT_EQ(request.get("other"), "trob");
    ASSERT_EQ(request.get("fake"), "");
}

TEST(HttpRequestTest, post_request_parse)
{
    const std::string raw_request =
            "POST /index.html HTTP/1.1\r\n"
            "\r\n"
            "post_data=data1&some_more_post_data=data23\r\n\r\n";

    //Parse it
    fr::HttpRequest request;
    ASSERT_EQ(request.parse(raw_request.c_str(), raw_request.size()), fr::Socket::Success);

    //Check that the request type is intact
    ASSERT_EQ(request.get_type(), fr::Http::Post);

    //Test that URI is intact
    ASSERT_EQ(request.get_uri(), "/index.html");

    //Parse code is the same for GET, so skip header checks. Test if POST data exists.
    ASSERT_EQ(request.post_exists("post_data"), true);
    ASSERT_EQ(request.post_exists("some_more_post_data"), true);
    ASSERT_EQ(request.post_exists("non_existant"), false);

    //Check that the POST data is valid
    ASSERT_EQ(request.post("post_data"), "data1");
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

TEST(HttpRequestTest, get_request_construction)
{
    //Create a request
    fr::HttpRequest request;
    ASSERT_EQ(request.get_uri(), "/");

    request.header("MyHeader") = "header1";
    request.header("MyOther-Header") = "header2";
    request.get("my_get") = "var1";
    request.get("my_other_get") = "var2";
    request.set_uri("heyo/bobby");
    request.set_type(fr::Http::Get);
    const std::string constructed_request = request.construct("frednicolson.co.uk");

    //Parse it and check that everything's correct
    request = {};
    request.parse(constructed_request.c_str(), constructed_request.size());
    ASSERT_EQ(request.header("MyHeader"), "header1");
    ASSERT_EQ(request.header("MyOther-Header"), "header2");
    ASSERT_EQ(request.get("my_get"), "var1");
    ASSERT_EQ(request.get("my_other_get"), "var2");
    ASSERT_EQ(request.get_uri(), "/heyo/bobby");
    ASSERT_EQ(request.get_type(), fr::Http::Get);
}

TEST(HttpRequestTest, post_request_construction)
{
    //Create a request
    fr::HttpRequest request;
    request.header("MyHeader") = "header1";
    request.header("myotherheader") = "header2";
    request.set_uri("/heyo/bobby");
    request.get("var") = "20";
    request.post("my_post") = "post_data";
    request.post("some_post") = "more_post";
    request.set_type(fr::Http::Post);
    const std::string constructed_request = request.construct("frednicolson.co.uk");

    //Parse it
    request = {};
    request.parse(constructed_request.c_str(), constructed_request.size());
    ASSERT_EQ(request.header("myheader"), "header1");
    ASSERT_EQ(request.header("MyOtherHeader"), "header2");
    ASSERT_EQ(request.get_uri(), "/heyo/bobby");
    ASSERT_EQ(request.get("var"), "20");
    ASSERT_EQ(request.post("my_post"), "post_data");
    ASSERT_EQ(request.post("some_post"), "more_post");
    ASSERT_EQ(request.get_type(), fr::Http::Post);
}

TEST(HttpRequestTest, partial_parse)
{
    //The test request to parse
    const std::string raw_request1 =
            "GET /index.html?var=bob&other=trob HTTP/1.1\n"
                    "Host: frednicolson.co.uk\r\n"
                    "Content-Type: application/x-www-form-urlencoded\r\n"
                    "My-Header:   ";

    const std::string raw_request2 =
            " header1\n"
            "My-Other-Header:header2\r\n"
            "Cache-Control: no-cache\r\n\r\n";


    //Parse part 1
    fr::HttpRequest request;
    ASSERT_EQ(request.parse(raw_request1.c_str(), raw_request1.size()), fr::Socket::NotEnoughData);

    //Parse part 2
    ASSERT_EQ(request.parse(raw_request2.c_str(), raw_request2.size()), fr::Socket::Success);

    //Verify it
    ASSERT_EQ(request.get_type(), fr::Http::Get);
    ASSERT_EQ(request.header("content-type"), "application/x-www-form-urlencoded");
    ASSERT_EQ(request.header("Cache-Control"), "no-cache");
}