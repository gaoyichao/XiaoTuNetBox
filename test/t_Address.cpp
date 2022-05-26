#include <gtest/gtest.h>
#include <iostream>

#include <XiaoTuNetBox/Address.h>

using namespace xiaotu;

TEST(Address, ipv4)
{
    net::IPv4 addr;
    EXPECT_EQ(AF_INET, addr.GetFamily());
    EXPECT_EQ("0.0.0.0", addr.GetIp());
    EXPECT_EQ(0, addr.GetPort());
    EXPECT_EQ("0.0.0.0:0", addr.GetIpPort());

    net::IPv4 addr1("127.0.0.1", 1234);
    EXPECT_EQ(AF_INET, addr1.GetFamily());
    EXPECT_EQ("127.0.0.1", addr1.GetIp());
    EXPECT_EQ(1234, addr1.GetPort());
    EXPECT_EQ("127.0.0.1:1234", addr1.GetIpPort());

    std::cout << "\u1F600" << std::endl;    
}


