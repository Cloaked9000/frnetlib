//
// Created by fred on 05/06/17.
//

#include <gtest/gtest.h>
#include <frnetlib/TcpListener.h>
#include <thread>

TEST(TcpListenerTest, listner_listen)
{
    fr::TcpListener listener;
    ASSERT_EQ(listener.get_socket_descriptor(), -1);
    fr::Socket::Status ret = listener.listen("9090");
    ASSERT_EQ(ret, fr::Socket::Success);
    listener.close_socket();
    ASSERT_EQ(listener.get_socket_descriptor(), -1);
}


TEST(TcpListenerTest, listener_accept)
{
    fr::TcpListener listener;
    listener.set_inet_version(fr::Socket::IP::v4);
    if(listener.listen("9095") != fr::Socket::Success)
        FAIL();

    auto client_thread = []()
    {
        fr::TcpSocket socket;
        socket.set_inet_version(fr::Socket::IP::v4);
        auto ret = socket.connect("127.0.0.1", "9095");
        ASSERT_EQ(ret, fr::Socket::Success);
    };

    std::thread t1(client_thread);
    fr::TcpSocket socket;
    auto ret = listener.accept(socket);
    ASSERT_EQ(ret, fr::Socket::Success);
    t1.join();
}

TEST(TcpListenerTest, set_descriptor)
{
    fr::TcpListener listener;
    listener.set_socket_descriptor(-20);
    ASSERT_EQ(listener.get_socket_descriptor(), -20);
}
