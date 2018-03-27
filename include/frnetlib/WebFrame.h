//
// Created by fred on 01/03/18.
//

#ifndef FRNETLIB_WEBFRAME_H
#define FRNETLIB_WEBFRAME_H


#include "Sendable.h"

namespace fr
{
    class WebFrame : public fr::Sendable
    {
    public:
        enum Opcode : uint8_t
        {
            Continuation = 0,
            Text = 1,
            Binary = 2,
            Disconnect = 8,
            Ping = 9,
            Pong = 10
        };

        /*!
         * Constructs the WebFrame.
         *
         * @param type The opcode type. See set_opcode. Text by default.
         */
        explicit WebFrame(Opcode type = Text);

        /*!
         * Get's the received payload data. (Data received).
         *
         * @return The payload
         */
        inline const std::string get_payload()
        {
            return payload;
        }

        /*!
         * Sets the frame payload (data being sent)
         *
         * @param payload_ The payload to send
         */
        inline void set_payload(std::string payload_)
        {
            payload = std::move(payload_);
        }

        /*!
         * Sets the WebFrame opcode (it's type)
         * This should be Text for non-binary data.
         * Or Binary for binary data.
         * Or Continuation if it's the next part of a fragmented message (set_final() set to true if this is the last part)
         *
         * @param opcode_ The opcode to use.
         */
        inline void set_opcode(Opcode opcode_)
        {
            opcode = opcode_;
        }

        /*!
         * Gets the WebFrame opcode (it's type)
         *
         * @return The opcode of the frame
         */
        inline Opcode get_opcode()
        {
            return opcode;
        }

        /*!
         * Checks if the frame is the final part of the message
         *
         * @return True if this frame is the final part, false if there's more to come.
         */
        inline bool is_final()
        {
            return final;
        }

        /*!
         * Sets whether the frame is the final part of a message or not.
         *
         * @param is_final True if this frame is the final part of a message. False if it's a fragment with more to come.
         */
        inline void set_final(bool is_final = true)
        {
            final = is_final;
        }

    protected:
        /*!
         * Overridable send, to allow
         * custom types to be directly sent through
         * sockets.
         *
         * @param socket The socket to send through
         * @return Status indicating if the send succeeded or not.
         */
        Socket::Status send(Socket *socket) const override;

        /*!
         * Overrideable receive, to allow
         * custom types to be directly received through
         * sockets.
         *
         * @note If the maximum message length is exceeded, then the connection will be closed
         * @param socket The socket to send through
         * @return Status indicating if the send succeeded or not.
         */
        Socket::Status receive(Socket *socket) override;

    private:
        mutable std::string payload;
        Opcode opcode;
        bool final;
        static uint32_t current_mask_key;
    };
}


#endif //FRNETLIB_WEBFRAME_H
