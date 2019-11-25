//
// Created by fred.nicolson on 25/09/17.
//

#include <gtest/gtest.h>
#include <frnetlib/HttpResponse.h>

TEST(HttpResponseTest, response_parse_v1)
{
    const std::string raw_response =
            "HTTP/1.0 301 Moved Permanently\n"
            "Server: nginx/1.10.2\n"
            "Date: Mon, 25 Sep 2017 13:51:56 GMT\n"
            "Content-Type: text/html\n"
            "Content-Length: 0\n"
            "Connection: keep-alive\n"
            "Location: https://frednicolson.co.uk/\n\n";

    //Parse response
    fr::HttpResponse test;
    ASSERT_EQ(test.parse(raw_response.c_str(), raw_response.size()), fr::Socket::Status::Success);

    //Verify it
    ASSERT_EQ(test.get_version(), fr::Http::RequestVersion::V1);
}

TEST(HttpResponseTest, response_parse_v2)
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
    ASSERT_EQ(test.parse(raw_response.c_str(), raw_response.size()), fr::Socket::Status::Success);

    //Verify it
    ASSERT_EQ(test.get_version(), fr::Http::RequestVersion::V1_1);
    ASSERT_EQ(test.get_status(), fr::Http::RequestStatus::MovedPermanently);
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
    ASSERT_EQ(test.parse(raw_response1.c_str(), raw_response1.size()), fr::Socket::Status::NotEnoughData);
    ASSERT_EQ(test.parse(raw_response2.c_str(), raw_response2.size()), fr::Socket::Status::NotEnoughData);
    ASSERT_EQ(test.parse(raw_response3.c_str(), raw_response3.size()), fr::Socket::Status::Success);

    //Verify it
    ASSERT_EQ(test.get_status(), fr::Http::RequestStatus::MovedPermanently);
    ASSERT_EQ(test.header("Content-length"), "177");
    ASSERT_EQ(test.get_body(), response_body);
}

TEST(HttpResponseTest, parse_chunked_response_test)
{
    const std::string raw_response1 =
                                    "HTTP/1.1 200 OK\r\n"
                                    "Content-Type: text/plain\r\n"
                                    "Transfer-Encoding: chunked\r\n"
                                    "\r\n"
                                    "7\r\n"
                                    "Mozilla\r\n"
                                    "9\r\n"
                                    "Developer\r\n"
                                    "7\r\n"
                                    "Network\r\n"
                                    "0\r\n"
                                    "\r\n";

    //Parse response
    fr::HttpResponse test;
    ASSERT_EQ(test.parse(raw_response1.c_str(), raw_response1.size()), fr::Socket::Status::Success);

    //Verify it
    ASSERT_EQ(test.get_status(), fr::Http::RequestStatus::Ok);
    ASSERT_EQ(test.get_body(), "MozillaDeveloperNetwork");
}

TEST(HttpResponseTest, parse_partial_chunked_response_test)
{
    const std::string raw_response1 =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Transfer-Encoding: chunked\r\n";

    const std::string raw_response2 =
            "\r\n"
            "7\r\n"
            "Mozilla\r\n";

    const std::string raw_response3 =
            "9\r\n";

    const std::string raw_response4 =
            "Developer\r\n"
            "7\r\n"
            "Netw";
    const std::string raw_response5 =
            "ork\r\n"
            "0\r\n";

    const std::string raw_response6 =
            "\r\n";

    //Parse response
    fr::HttpResponse test;
    ASSERT_EQ(test.parse(raw_response1.c_str(), raw_response1.size()), fr::Socket::Status::NotEnoughData);
    ASSERT_EQ(test.parse(raw_response2.c_str(), raw_response2.size()), fr::Socket::Status::NotEnoughData);
    ASSERT_EQ(test.parse(raw_response3.c_str(), raw_response3.size()), fr::Socket::Status::NotEnoughData);
    ASSERT_EQ(test.parse(raw_response4.c_str(), raw_response4.size()), fr::Socket::Status::NotEnoughData);
    ASSERT_EQ(test.parse(raw_response5.c_str(), raw_response5.size()), fr::Socket::Status::NotEnoughData);
    ASSERT_EQ(test.parse(raw_response6.c_str(), raw_response6.size()), fr::Socket::Status::Success);

    //Verify it

    ASSERT_EQ(test.get_status(), fr::Http::RequestStatus::Ok);
    ASSERT_EQ(test.get_body(), "MozillaDeveloperNetwork");
}

TEST(HttpResponseTest, header_length_test)
{
    //Try data with no header end first
    std::string buff(MAX_HTTP_HEADER_SIZE + 1, '\0');
    fr::HttpResponse response;
    buff.insert(0, "HTTP");
    ASSERT_EQ(response.parse(buff.c_str(), buff.size()), fr::Socket::Status::HttpHeaderTooBig);
    response = {};

    //Now try short header but long data, this should work
    buff = "HTTP/1.1 301 Moved Permanently\n"
                    "Content-Type: text/html\n"
                    "Content-Length: " + std::to_string(MAX_HTTP_BODY_SIZE - 1) + "\n"
                    "Connection: keep-alive\n"
                    "\n" + std::string(MAX_HTTP_BODY_SIZE - 1, '\0');
    ASSERT_EQ(response.parse(buff.c_str(), buff.size()), fr::Socket::Status::Success);
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
    ASSERT_EQ(response.parse(buff.c_str(), buff.size()), fr::Socket::Status::HttpBodyTooBig);
}

TEST(HttpResponseTest, HttpResponseConstruction)
{
    {
        fr::HttpResponse response;
        response.set_status(fr::Http::RequestStatus::ImATeapot);
        response.header("bob") = "trob";
        response.set_body("lob");
        auto constructed = response.construct("frednicolson.co.uk");
        response = {};
        ASSERT_EQ(response.parse(constructed.c_str(), constructed.size()), fr::Socket::Status::Success);

        ASSERT_EQ(response.get_version(), fr::Http::RequestVersion::V1_1);
        ASSERT_EQ(response.get_status(), fr::Http::RequestStatus::ImATeapot);
        ASSERT_EQ(response.get_body(), "lob");
        ASSERT_EQ(response.header("bob"), "trob");
    }

    {
        fr::HttpResponse response;
        response.set_version(fr::Http::RequestVersion::V1);
        auto constructed = response.construct("frednicolson.co.uk");
        response = {};
        response.parse(constructed.c_str(), constructed.size());

        ASSERT_EQ(response.get_version(), fr::Http::RequestVersion::V1);
    }

}