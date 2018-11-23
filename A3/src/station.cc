#include <iostream>

#include "station.h"

#include "monitor.h"
#include "collidable.h"
#include "slot_finder.h"
#include "packet.h"

namespace vs {

void Station::run(const std::string& group, uint16_t port, const std::string& iface, char type)
{
	typedef Collideable<std::pair<uint64_t, packet_t>> receiver_t;
	typedef SlotFinder<SLOTS_PER_FRAME> finder_t;

	Station self(iface, port);
	Schedule<SLOTS_PER_FRAME> schedule;
	finder_t find_slot;
	Averager<int64_t> timer;
	Monitor<payload_t> payload;
	Monitor<receiver_t> received;
	packet_t my_packet;

	try
	{
		my_packet.type = type;

		self.mSender.join(group, iface);
		self.mReceiver.join(group);

		self.mReader = [&](void) {
			payload_t buf;

			while(true)
			{
				for(uint i = 0 ; i < sizeof(buf) ; ++i)
				{
					buf[i] = std::cin.get();
				}

				payload.set(buf);
			}
		};

		self.mListener = [&](void) {
			self.mReceiver.listen([&](const Socket::buffer_t& msg) {
				const packet_t& packet(*reinterpret_cast<const packet_t *>(&msg[0]));

				received.set(std::make_pair(clock_t::now(), packet));
			});
		};

		clock_t::sync(FRAME_DURATION);

		int my_slot = -1;

		for(uint64_t frame = (clock_t::now() + FRAME_DURATION / 2) / FRAME_DURATION ; true ; ++frame)
		{
			uint64_t frame_start = frame * FRAME_DURATION;

			for(int slot = 0 ; slot < SLOTS_PER_FRAME ; ++slot)
			{
				uint64_t slot_start = frame_start + slot * SLOT_DURATION;

				clock_t::sleep_until(slot_start);

				received.access([&](receiver_t& o) {
					if(o.isPresent() && !o.hasCollided())
					{
						auto p(static_cast<receiver_t::value_type>(o));
						uint64_t t = p.first;
						packet_t& packet(p.second);

						if(packet.type == 'A')
						{
							timer.add(t - packet.timestamp);
						}

						if(packet.timestamp / FRAME_DURATION == frame)
						{
							schedule.set(packet.next_slot - SLOT_IDX_OFFSET);
						}
					}
					else
					{
						o.clear();
					}
				});

				if(slot == my_slot) try
				{
					my_packet.payload = payload.get();
					my_packet.next_slot = find_slot(schedule) + SLOT_IDX_OFFSET;
					my_packet.timestamp = slot_start + SLOT_DURATION / 2;

					clock_t::sleep_until(my_packet.timestamp);

					if(difference(clock_t::now(), my_packet.timestamp) < SLOT_DURATION / 4)
					{
						self.mSender.send(group, port, &my_packet, sizeof(my_packet));
					}
				}
				catch(const finder_t::NoEmptySlotError& e)
				{
					my_slot = -1;
				}

				if(slot == 0)
				{
					schedule.clear();
				}
  			}

			my_slot = (my_slot == -1) ? find_slot(schedule) : (my_packet.next_slot - SLOT_IDX_OFFSET);

			clock_t::adjust(-timer.get() / 2);

			clock_t::sleep_until(frame_start + FRAME_DURATION);
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;

		throw;
	}
}

Station::Station(const std::string& iface, uint16_t port)
	: mSender(Socket::getAddressOf(iface))
	, mReceiver(port)
{
}

Station::~Station(void)
{
}

}

