#ifndef VS_PACKET_H
#define VS_PACKET_H

#include "util.h"

namespace vs
{
	typedef struct __attribute__((packed))
	{
		char type;
		char name[10];
		char payload[14];
		byte_t next_slot;
		uint64_t timestamp;
	} packet_t;
}

#endif

