#include <iostream>

#include "sniffer.h"

#include "clock.h"
#include "socket.h"

namespace vs {

void Sniffer::start(const std::string& group, uint16_t port)
{
	mSock.reset(new Socket(port));
	mSock->join(group);

	mThread = std::thread([this](void) {
		mSock->listen([this](const Socket::buffer_t& msg) {
			mCallback(Packet{msg});
		});
	});
}

void Sniffer::stop(void)
{
	if(!static_cast<bool>(mSock))
		THROW<InvalidStateError>("Not currently running!");

	mSock.reset();
	mThread.join();
}

void Sniffer::run(const std::string& group, uint16_t port)
{
	typedef Clock<> clock_t;

	const uint delta = SLOT_DURATION / 3;

	uint64_t last_frame = 0;
	uint last_slot = SLOTS_PER_FRAME;

	Sniffer snif([&](const Packet& msg) {
		uint64_t t = clock_t::now();
		uint64_t frame = (t + FRAME_DURATION - 1) / FRAME_DURATION;
		uint slot = (t % FRAME_DURATION) / SLOT_DURATION;
		std::string teamname(msg.payload(), msg.payload() + NAME_LENGTH);
		uint offset = ((t % FRAME_DURATION) % SLOT_DURATION);

		bool collision = (last_frame == frame && last_slot == slot);
		bool middle = ((delta < offset) && (offset < SLOT_DURATION - delta));
		bool accurate = (difference(t, msg.timestamp()) < SLOT_DURATION / 2);

		if(last_frame != frame)
		{
			std::cout << stringify("=== Frame ", std::setw(12), frame, " ===\n")
					  << std::flush;
		}

		last_slot = slot;
		last_frame = frame;

		std::cout << stringify(
			(collision ? 'X' : '-'),
			(!middle ? 'O' : '-'),
			(!accurate ? 'A' : '-'),
			' ', std::setw(14), t,
			" (Slot ",
			std::setw(2), slot,
			"): received '", teamname,
			"' [", msg.type(), "] next slot: ",
			std::setw(2), msg.next_slot(),
			" TX: ", msg.timestamp(), "\n")
		<< std::flush;
	});

	snif.start(group, port);

	for(std::string line ; std::getline(std::cin, line) ;)
	{
		if(line.empty()) continue;

		if(line[0] == 'q') break;
	}

	snif.stop();
}

}

