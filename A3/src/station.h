#ifndef VS_STATION_H
#define VS_STATION_H

#include "util.h"
#include "clock.h"
#include "schedule.h"
#include "thread.h"
#include "socket.h"

namespace vs
{
	class Station
	{
		typedef Clock<> clock_t;

		public:
			static void run(const std::string&, uint16_t, const std::string&, char);

		private:
			Station(const std::string&, uint16_t);
			~Station( );

		private:
			Thread mReader, mListener;
			Socket mSender, mReceiver;
	};
}

#endif

