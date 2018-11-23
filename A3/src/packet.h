#ifndef VS_PACKET_H
#define VS_PACKET_H

#include <array>

#include "util.h"

namespace vs
{
	typedef std::array<byte_t, PAYLOAD_SIZE> payload_t;

	typedef struct __attribute__((packed))
	{
		char type;
		payload_t payload;
		byte_t next_slot;
		uint64_t timestamp;
	} packet_t;

	static_assert(sizeof(packet_t) == PACKET_SIZE);
}

#endif

