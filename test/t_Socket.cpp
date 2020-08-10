#include <gtest/gtest.h>
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>

#include <XiaoTuNetBox/Socket.h>

using namespace xiaotu;

TEST(Socket, init)
{
    net::Socket s(0);
    EXPECT_EQ(0, s.GetFd());

    net::Socket s1(AF_INET, SOCK_STREAM, 0);
    EXPECT_NE(-1, s1.GetFd());
}


