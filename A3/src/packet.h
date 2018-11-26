#ifndef VS_PACKET_H
#define VS_PACKET_H

#include <array>
#include <cstring>

#include "util.h"
#include "socket.h"

namespace vs
{
	typedef std::array<byte_t, PAYLOAD_SIZE> payload_t;

	class Packet
	{
		typedef struct __attribute__((packed))
		{
			char type;
			payload_t payload;
			byte_t next_slot;
			uint64_t timestamp;
		} packet_t;

		static_assert(sizeof(packet_t) == PACKET_SIZE);

		static constexpr int OFFSET = 1;
		static constexpr uint64_t swap_bytes(uint64_t v) { return
			(((v >>  0) & 0xff) << 56) | 
			(((v >>  8) & 0xff) << 48) |
			(((v >> 16) & 0xff) << 40) |
			(((v >> 24) & 0xff) << 32) | 
			(((v >> 32) & 0xff) << 24) | 
			(((v >> 40) & 0xff) << 16) | 
			(((v >> 48) & 0xff) <<  8) | 
			(((v >> 56) & 0xff) <<  0);
		}

		public:
			Packet( ) { std::memset(&mData, 0, sizeof(mData)); }
			explicit Packet(const Socket::buffer_t& b) { std::memcpy(&mData, &b[0], b.size()); }

			char type( ) const  { return mData.type; }
			void type(char t) { mData.type = t; }
			const byte_t *payload( ) const { return &mData.payload[0]; }
			byte_t *payload( ) { return &mData.payload[0]; }
			void payload(const payload_t& p) { mData.payload = p; }
			int next_slot( ) const { return mData.next_slot - OFFSET; }
			void next_slot(byte_t s) { mData.next_slot = s + OFFSET; }
			uint64_t timestamp( ) const { return swap_bytes(mData.timestamp); }
			void timestamp(uint64_t tx) { mData.timestamp = swap_bytes(tx); }

			const byte_t *raw( ) const { return reinterpret_cast<const byte_t *>(&mData); }

		private:
			packet_t mData;
	};
}

#endif

