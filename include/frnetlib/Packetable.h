//
// Created by fred on 02/02/17.
//

#ifndef FRNETLIB_PACKETABLE_H
#define FRNETLIB_PACKETABLE_H
#include <string>

namespace fr
{
    class Packet;
    class Packetable
    {
    public:
        virtual ~Packetable() = default;

        /*!
         * Called to pack class data into the 'destination'
         * packet.
         *
         * @param destination Where you should store the class data
         */
        virtual void pack(fr::Packet &destination) const=0;

        /*!
         * Called to unpack class data from the 'source' packet.
         *
         * @param source Where to retreive the class data from.
         */
        virtual void unpack(fr::Packet &source)=0;
    };
}

#endif //FRNETLIB_PACKETABLE_H
