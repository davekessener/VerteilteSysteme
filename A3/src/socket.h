#ifndef VS_SOCKET_H
#define VS_SOCKET_H

#include <functional>
#include <vector>

#include "util.h"
#include "error.h"

namespace vs
{
	class Socket
	{
		public:
		typedef std::vector<byte_t> buffer_t;
		typedef std::function<void(const buffer_t&)> callback_fn;

		DEFINE_EXCEPTION(InterruptedError);
		DEFINE_EXCEPTION(InvalidStateError);

		public:
			Socket(uint16_t = 0);
			~Socket( );
			void join(const std::string&);
			void send(const std::string&, uint16_t, const void *, size_t);
			void listen(callback_fn) const;
			buffer_t receive( ) const;

			bool open( ) const;

		private:
			struct Impl;

			Impl * const self;
	};
}

#endif

