#include <algorithm>
#include "gtest/gtest.h"
#include <frnetlib/Packetable.h>
#include <frnetlib/Packet.h>

class CustomClass : public fr::Packetable
{
public:
    CustomClass()
    {
        member1 = "hello";
        member2 = 7;
    }

    bool operator==(const CustomClass &o) const
    {
        return o.member1 == member1 && o.member2 == member2;
    }

    virtual void pack(fr::Packet &destination) const
    {
        destination << member1 << member2;
    }

    virtual void unpack(fr::Packet &source)
    {
        source >> member1 >> member2;
    }

private:
    std::string member1;
    int member2;
};

TEST(PacketableTest, pack_and_unpack)
{
    fr::Packet packet;
    CustomClass custom;
    packet << custom;

    CustomClass custom2;
    packet >> custom2;

    ASSERT_EQ(custom, custom2);
}
