//
// Created by fred on 01/03/18.
//

#include "frnetlib/WebFrame.h"
#include "frnetlib/WebSocket.h"

namespace fr
{
    uint32_t WebFrame::current_mask_key = static_cast<uint32_t>(std::time(nullptr));

    WebFrame::WebFrame(WebFrame::Opcode type)
    : opcode(type),
      final(true)
    {

    }

    fr::Socket::Status WebFrame::send(Socket *socket_) const
    {
        auto *socket = dynamic_cast<WebSocketBase*>(socket_);
        if(!socket)
            return Socket::Error;

        uint16_t first_2bytes = 0;
        std::string buffer;

        //Set fin bit. Bit 1.
        first_2bytes |= final << 15;

        //Set opcode bit
        first_2bytes |= opcode << 8;

        //Set mask bit (dependent on is_client flag, only client -> server messages are masked)
        first_2bytes |= socket->is_client() << 7;

        //Set payload length
        if(payload.size() <= 125)
            first_2bytes |= payload.size();
        else
            first_2bytes |= (payload.size() < std::numeric_limits<uint16_t>::max()) ? 126 : 127;
        first_2bytes = htons(first_2bytes);
        buffer.append((char*)&first_2bytes, sizeof(first_2bytes));

        //Set additional payload bits if large enough
        if(payload.size() > 125)
        {
            if(payload.size() < std::numeric_limits<uint16_t>::max()) //16bit length
            {
                auto len = htons(static_cast<uint16_t>(payload.size()));
                buffer.append((char*)&len, sizeof(len));
            }
            else //64bit length
            {
                uint64_t len = fr_htonll(payload.size());
                buffer.append((char*)&len, sizeof(len));
            }
        }

        //Add a masking key if we're the client
        if(socket->is_client())
        {
            union
            {
                uint32_t mask_key;
                char str_mask_key[4];
            } mask_union{};

            mask_union.mask_key = ++current_mask_key;
            buffer.append((char*)&mask_union.mask_key, sizeof(mask_union.mask_key));

            //Encode the payload using the mask key
            for(size_t a = 0; a < payload.size(); ++a)
            {
                payload[a] = payload[a] ^ mask_union.str_mask_key[a % 4];
            }
        }

        buffer.append(payload);
        return socket_->send_raw(buffer.c_str(), buffer.size());
    }

    Socket::Status WebFrame::receive(Socket *socket)
    {
        auto *socket_ = dynamic_cast<WebSocketBase*>(socket);
        if(!socket_)
            return Socket::Error;
        payload.clear();
        Socket::Status status;

        uint16_t first_2bytes;
        status = socket->receive_all(&first_2bytes, sizeof(first_2bytes));
        if(status != fr::Socket::Success)
            return status;
        first_2bytes = ntohs(first_2bytes);

        //Extract fin bit. Read bit 1.
        final = static_cast<bool>((first_2bytes >> 15) & 0x1);

        //Extract opcode. Read bits 4-7
        opcode = static_cast<Opcode>((first_2bytes >> 8) & 0xF);

        //Extract mask, if we're the server then messages should always be masked. Read bit 9
        auto mask = static_cast<bool>((first_2bytes >> 7) & 0x1);
        if(mask == socket_->is_client())
        {
            return fr::Socket::Error;
        }


        //Extract payload length. Read bits 9-15
        auto payload_length = static_cast<uint64_t>(first_2bytes & 0x7F);
        if(payload_length == 126) //Length is longer than 7 bit, so read 16bit length
        {
            uint16_t length;
            status = socket->receive_all(&length, sizeof(length));
            payload_length = ntohs(length);
            if(status != fr::Socket::Success)
                return status;
        }
        else if(payload_length == 127) //Length is longer than 16 bit, so read 64bit length
        {
            status = socket->receive_all(&payload_length, sizeof(payload_length));
            payload_length = fr_ntohll(payload_length);
            if(status != fr::Socket::Success)
                return status;
        }

        //Verify that payload length isn't too large
        if(socket->get_max_receive_size() && payload_length > socket->get_max_receive_size())
        {
            return Socket::MaxPacketSizeExceeded;
        }

        //Read masking key if the mask bit is set
        union
        {
            uint32_t mask_key;
            char str_mask_key[4];
        } mask_union{};
        if(mask)
        {
            status = socket->receive_all(&mask_union.mask_key, 4);
            if(status != fr::Socket::Success)
                return status;
        }

        //Read payload
        payload.resize(payload_length, '\0');
        status = socket->receive_all(&payload[0], payload_length);
        if(status != fr::Socket::Success)
            return status;

        //Decode the payload if the mask bit is set
        if(mask)
        {
            for(size_t a = 0; a < payload_length; ++a)
            {
                payload[a] = payload[a] ^ mask_union.str_mask_key[a % 4];
            }

        }
        return fr::Socket::Success;
    }

}

