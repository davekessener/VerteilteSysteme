#include <iostream>

#include "station.h"

#include "monitor.h"
#include "collidable.h"
#include "slot_finder.h"
#include "packet.h"

namespace vs {

namespace
{
	class Station
	{
		typedef Clock<> clock_t;
		typedef Collideable<std::pair<uint64_t, Packet>> receiver_t;
		typedef SlotFinder<SLOTS_PER_FRAME> finder_t;

		public:
			Station(const std::string&, uint16_t, const std::string&, char);
			~Station( );
			void run( );

		private:
			void receive( );
			void listen( );
			void step(uint64_t, int);
			void send(uint64_t);
			void prepare(uint64_t);

		private:
			const std::string mGroup, mIface;
			const uint16_t mPort;
			Thread mListen, mReceive;
			Socket mSender, mReceiver;
			Schedule<SLOTS_PER_FRAME> mSchedule;
			finder_t mFindSlot;
			Averager<int64_t> mTimer;
			Monitor<payload_t> mPayload;
			Monitor<receiver_t> mReceived;
			Packet mPacket;
			int mSlot;
	};
}

// # ==============================================================================================

void runStation(const std::string& group, uint16_t port, const std::string& iface, char type)
{
	Station station(group, port, iface, type);

	station.run();
}

// # ----------------------------------------------------------------------------------------------

Station::Station(const std::string& group, uint16_t port, const std::string& iface, char type)
	: mGroup(group)
	, mIface(iface)
	, mPort(port)
	, mSender(Socket::getAddressOf(iface))
	, mReceiver(port)
	, mSlot(-1)
{
	mPacket.type(type);
}

Station::~Station(void)
{
	mReceive.detach();
}

void Station::run(void)
{
	mSender.join(mGroup, mIface);
	mReceiver.join(mGroup);

	receive();
	listen();

	clock_t::sync(FRAME_DURATION);

	mSlot = -1;

	while(true)
	{
		uint64_t frame = (clock_t::now() + FRAME_DURATION / 2) / FRAME_DURATION;
		uint64_t frame_start = frame * FRAME_DURATION;

		mSchedule.clear();

		for(int slot = 0 ; slot < SLOTS_PER_FRAME ; ++slot)
		{
			uint64_t slot_start = frame_start + slot * SLOT_DURATION;
			uint64_t slot_end = slot_start + SLOT_DURATION;

			if(slot == mSlot)
			{
				send((slot_start + slot_end) / 2);
			}

			clock_t::sleep_until(slot_end);

			step(frame, slot);
		}

		mSlot = (mSlot == -1) ? mFindSlot(mSchedule) : mPacket.next_slot();

		clock_t::adjust(-mTimer.get() / 2);
	}
}

void Station::receive(void)
{
	mReceive = [this](void) {
		payload_t buffer;

		while(true)
		{
			for(uint i = 0 ; i < sizeof(buffer) ; ++i)
			{
				buffer[i] = std::cin.get();
			}

			mPayload.set(buffer);
		}
	};
}

void Station::listen(void)
{
	mListen = [this](void) {
		mReceiver.listen([this](const Socket::buffer_t& msg) {
			mReceived.set(std::make_pair(clock_t::now(), Packet{msg}));
		});
	};
}

void Station::send(uint64_t tx)
{
	try
	{
		prepare(tx);

		clock_t::sleep_until(tx);

		mSender.send(mGroup, mPort, mPacket.raw(), PACKET_SIZE);
	}
	catch(const finder_t::NoEmptySlotError& e)
	{
		mSlot = -1;
	}
}

void Station::step(uint64_t frame, int slot)
{
	mReceived.access([&](receiver_t& o) {
		if(o.isPresent() && !o.hasCollided())
		{
			auto p(static_cast<receiver_t::value_type>(o));
			uint64_t t = p.first;
			Packet& packet(p.second);

			if(packet.type() == 'A')
			{
				mTimer.add(t - packet.timestamp());
			}

			if(packet.timestamp() / FRAME_DURATION == frame)
			{
				mSchedule.set(packet.next_slot());
			}
		}
		else
		{
			o.clear();

			if(slot == mSlot)
			{
				mSlot = -1;
			}
		}
	});
}

void Station::prepare(uint64_t tx)
{
	mPacket.payload(mPayload.get());
	mPacket.next_slot(mFindSlot(mSchedule));
	mPacket.timestamp(tx);
}

}

