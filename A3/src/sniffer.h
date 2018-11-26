#ifndef VS_SNIFFER_H
#define VS_SNIFFER_H

#include <functional>
#include <thread>
#include <atomic>
#include <memory>

#include "packet.h"
#include "socket.h"

#include "error.h"

namespace vs
{
	class Sniffer
	{
		public:
		typedef std::function<void(const Packet&)> callback_fn;

		DEFINE_EXCEPTION(InvalidStateError);

		public:
			Sniffer(callback_fn f) : mCallback(f) { }
			void start(const std::string&, uint16_t);
			void stop( );

			static void run(const std::string&, uint16_t);

		private:
			callback_fn mCallback;
			std::thread mThread;
			std::unique_ptr<Socket> mSock;
	};
}

#endif

