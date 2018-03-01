//
// Created by fred.nicolson on 01/03/18.
//

#include <gtest/gtest.h>
#include <frnetlib/HttpResponse.h>

TEST(HttpTest, test_request_type_to_string)
{
    for(size_t a = 0; a < fr::Http::RequestTypeCount; ++a)
    {
        ASSERT_EQ(fr::Http::string_to_request_type(fr::Http::request_type_to_string((fr::Http::RequestType)a)), a);
    }

    ASSERT_EQ(fr::Http::request_type_to_string(fr::Http::Partial), "UNKNOWN");
    ASSERT_EQ(fr::Http::request_type_to_string(fr::Http::RequestTypeCount), "UNKNOWN");
    ASSERT_EQ(fr::Http::request_type_to_string(fr::Http::Unknown), "UNKNOWN");
}

TEST(HttpTest, test_string_to_request_type)
{
    std::vector<std::pair<fr::Http::RequestType, std::string>> strings = {
            {fr::Http::Get, "GET"},
            {fr::Http::Put, "PUT"},
            {fr::Http::Delete, "DELETE"},
            {fr::Http::Patch, "PATCH"},
            {fr::Http::Patch, "PATCHid-=wa"},
            {fr::Http::Partial, "PA"},
            {fr::Http::Partial, "PU"},
            {fr::Http::Partial, "DELET"},
            {fr::Http::Unknown, "DELETa"},
            {fr::Http::Unknown, "U"},
            {fr::Http::Unknown, "dwaouidhwi"},
            {fr::Http::Unknown, "get"},
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