#include "sniffer.h"

#include "socket.h"

namespace vs {

void Sniffer::start(const std::string& group, uint16_t port)
{
	mSock.reset(new Socket(port));
	mSock->join(group);

	mThread = std::thread([this](void) {
		mSock->listen([this](const Socket::buffer_t& msg) {
			mCallback(*reinterpret_cast<const packet_t *>(&msg[0]));
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

}

