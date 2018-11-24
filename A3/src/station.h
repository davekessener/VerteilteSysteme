#ifndef VS_STATION_H
#define VS_STATION_H

#include "util.h"
#include "clock.h"
#include "schedule.h"
#include "thread.h"
#include "socket.h"

namespace vs
{
	void runStation(const std::string&, uint16_t, const std::string&, char);
}

#endif

