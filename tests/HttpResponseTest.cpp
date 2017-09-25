//
// Created by fred.nicolson on 25/09/17.
//

#include <gtest/gtest.h>
#include <frnetlib/HttpResponse.h>

TEST(HttpResponseTest, response_parse)
{
    const std::string raw_response =
            "HTTP/1.1 301 Moved Permanently\n"
            "Server: nginx/1.10.2\n"
            "Date: Mon, 25 Sep 2017 13:51:56 GMT\n"
            "Content-Type: text/html\n"
            "Content-Length: 177\n"
            "Connection: keep-alive\n"
            "Location: https://frednicolson.co.uk/\n"
            "\n"
            "<html>\n"
            "<head><title>301 Moved Permanently</title></head>\n"
            "<body bgcolor=\"white\">\n"
            "<center><h1>301 Moved Permanently</h1></center>\n"
            "<hr><center>nginx/1.10.2</center>\n"
            "</body>\n"
            "</html>";

    const std::string response_body =
            "<html>\n"
            "<head><title>301 Moved Permanently</title></head>\n"
            "<body bgcolor=\"white\">\n"
            "<center><h1>301 Moved Permanently</h1></center>\n"
            "<hr><center>nginx/1.10.2</center>\n"
            "</body>\n"
            "</html>";

    //Parse response
    fr::HttpResponse test;
    ASSERT_EQ(test.parse(raw_response.c_str(), raw_response.size()), fr::Socket::Success);

    //Verify it
    ASSERT_EQ(test.get_status(), fr::Http::MovedPermanently);
    ASSERT_EQ(test.header("Content-length"), "177");
    ASSERT_EQ(test.get_body(), response_body);
}

TEST(HttpResponseTest, response_partial_parse)
{
    const std::string raw_response1 =
            "HTTP/1.1 301 Moved Permanently\n"
                    "Server: nginx/1.10.2\n"
                    "Date: Mon, 25 Sep 2017 13:51:56 GMT\n"
                    "Content-Type: text/html\n"
                    "Content-Length: 177\n"
                    "Connection: keep-alive\n";

    std::string raw_response2 =
                    "Location: https://frednicolson.co.uk/\n"
                    "\n"
                    "<html>\n"
                    "<head><title>301 Moved Permanently</title></head>\n"
                    "<body bgcolor=\"white\">\n";

    std::string raw_response3 =
                    "<center><h1>301 Moved Permanently</h1></center>\n"
                    "<hr><center>nginx/1.10.2</center>\n"
                    "</body>\n"
                    "</html>";

    const std::string response_body =
            "<html>\n"
            "<head><title>301 Moved Permanently</title></head>\n"
            "<body bgcolor=\"white\">\n"
            "<center><h1>301 Moved Permanently</h1></center>\n"
            "<hr><center>nginx/1.10.2</center>\n"
            "</body>\n"
            "</html>";

    //Parse response
    fr::HttpResponse test;
    ASSERT_EQ(test.parse(raw_response1.c_str(), raw_response1.size()), fr::Socket::NotEnoughData);
    ASSERT_EQ(test.parse(raw_response2.c_str(), raw_response2.size()), fr::Socket::NotEnoughData);
    ASSERT_EQ(test.parse(raw_response3.c_str(), raw_response3.size()), fr::Socket::Success);

    //Verify it
    ASSERT_EQ(test.get_status(), fr::Http::MovedPermanently);
    ASSERT_EQ(test.header("Content-length"), "177");
    ASSERT_EQ(test.get_body(), response_body);
}

TEST(HttpResponseTest, header_length_test)
{
    //Try data with no header end first
    std::string buff(MAX_HTTP_HEADER_SIZE + 1, '\0');
    fr::HttpResponse response;
    ASSERT_EQ(response.parse(buff.c_str(), buff.size()), fr::Socket::HttpHeaderTooBig);
    response = {};

    //Now try short header but long data, this should work
    buff = "HTTP/1.1 301 Moved Permanently\n"
                    "Content-Type: text/html\n"
                    "Content-Length: " + std::to_string(MAX_HTTP_BODY_SIZE - 1) + "\n"
                    "Connection: keep-alive\n"
                    "\n" + std::string(MAX_HTTP_BODY_SIZE - 1, '\0');
    ASSERT_EQ(response.parse(buff.c_str(), buff.size()), fr::Socket::Success);
}

TEST(HttpResponseTest, body_length_test)
{
    std::string buff =
            "HTTP/1.1 301 Moved Permanently\n"
            "Content-Type: text/html\n"
            "Content-Length: " + std::to_string(MAX_HTTP_BODY_SIZE + 1) + "\n"
            "Connection: keep-alive\n"
            "\n";
    buff += std::string(MAX_HTTP_BODY_SIZE + 1, '\0');
    fr::HttpResponse response;
    ASSERT_EQ(response.parse(buff.c_str(), buff.size()), fr::Socket::HttpBodyTooBig);
}