//
// Created by fred.nicolson on 01/03/18.
//

#include <gtest/gtest.h>
#include <gmock/gmock-more-matchers.h>
#include <frnetlib/HttpResponse.h>

TEST(HttpTest, test_request_type_to_string)
{
    for(size_t a = 0; a < (uint32_t)fr::Http::RequestType::RequestTypeCount; ++a)
    {
        ASSERT_EQ((size_t)fr::Http::string_to_request_type(fr::Http::request_type_to_string((fr::Http::RequestType)a)), a);
    }

    ASSERT_EQ(fr::Http::request_type_to_string(fr::Http::RequestType::Partial), "UNKNOWN");
    ASSERT_EQ(fr::Http::request_type_to_string(fr::Http::RequestType::RequestTypeCount), "UNKNOWN");
    ASSERT_EQ(fr::Http::request_type_to_string(fr::Http::RequestType::Unknown), "UNKNOWN");
}

TEST(HttpTest, test_string_to_request_type)
{
    std::vector<std::pair<fr::Http::RequestType, std::string>> strings = {
            {fr::Http::RequestType::Get, "GET"},
            {fr::Http::RequestType::Put, "PUT"},
            {fr::Http::RequestType::Delete, "DELETE"},
            {fr::Http::RequestType::Patch, "PATCH"},
            {fr::Http::RequestType::Patch, "PATCHid-=wa"},
            {fr::Http::RequestType::Partial, "PA"},
            {fr::Http::RequestType::Partial, "PU"},
            {fr::Http::RequestType::Partial, "DELET"},
            {fr::Http::RequestType::Unknown, "DELETa"},
            {fr::Http::RequestType::Unknown, "U"},
            {fr::Http::RequestType::Unknown, "dwaouidhwi"},
            {fr::Http::RequestType::Unknown, "get"},
    };

    for(auto &str : strings)
    {
        ASSERT_EQ(fr::Http::string_to_request_type(str.second), str.first);
    }
}

TEST(HttpTest, test_url_encode)
{
    std::string source = "1\"!£FEW$\"931-90%%+-&*0(du%a90dj09=_da.A~";
    ASSERT_EQ(fr::Http::url_encode(source), "1%22!%C2%A3FEW%24%22931-90%25%25%2B-%26*0(du%25a90dj09%3D_da.A~");
}

TEST(HttpTest, test_url_decode)
{
    std::string source = "1%22!%C2%A3FEW%24%22931-90%25%25%2B-%26*0(du%25a90dj09%3D_da.A~";
    ASSERT_EQ(fr::Http::url_decode(source), "1\"!£FEW$\"931-90%%+-&*0(du%a90dj09=_da.A~");
}

TEST(HttpTest, test_get_mimetype)
{
    ASSERT_EQ(fr::Http::get_mimetype(".html"), "text/html");
    ASSERT_EQ(fr::Http::get_mimetype("my_file.html"), "text/html");
    ASSERT_EQ(fr::Http::get_mimetype("file.some_random_type"), "application/octet-stream");
}

TEST(HttpTest, test_string_to_transfer_encoding)
{
    ASSERT_EQ(fr::Http::string_to_transfer_encoding("chunked"), fr::Http::TransferEncoding::Chunked);
    ASSERT_EQ(fr::Http::string_to_transfer_encoding("unknown"), fr::Http::TransferEncoding::None);
    ASSERT_EQ(fr::Http::string_to_transfer_encoding("ioudhweauidhgiwuyahfiuywhafyuhgwayufhg"), fr::Http::TransferEncoding::None);
    ASSERT_EQ(fr::Http::string_to_transfer_encoding("IDenTITy"), fr::Http::TransferEncoding::Identity);
}

namespace fr
{
    TEST(HttpTest, test_string_split)
    {
        ASSERT_THAT(Http::split_string("chunked\nstuff"), ::testing::ElementsAre("chunked", "stuff"));
        ASSERT_THAT(Http::split_string("chunked\n stuff"), ::testing::ElementsAre("chunked", " stuff"));
        ASSERT_THAT(Http::split_string("chunked,stuff", ',', false), ::testing::ElementsAre("chunked", "stuff"));
        ASSERT_THAT(Http::split_string("chunked, stuff", ',', true), ::testing::ElementsAre("chunked", "stuff"));
    }
}