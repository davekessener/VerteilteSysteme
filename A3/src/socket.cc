#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include "socket.h"

#include "posix_error.h"

#define MXT_BUFSIZE (64*1024)
#define MXT_TTL 1
#define MXT_TIMEOUT {0,250000}
#define MXT_INVALID_ADDR ((in_addr_t)-1)

namespace vs {

struct Socket::Impl
{
	int sock;
	sockaddr_in addr;
	struct
	{
		std::string host = "";
		uint16_t port = 0;
	} last;
};

Socket::Socket(uint16_t port)
	: self(new Impl)
{
	auto& sock(self->sock);
	auto& addr(self->addr);

	sock = socket(AF_INET, SOCK_DGRAM, 0);

	if(sock < 0)
		THROW<PosixError>("Socket creation failed");

	int reuseport = 1;
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &reuseport, sizeof(reuseport)))
		THROW<PosixError>("Could not REUSEPORT");
	
	int ttl = MXT_TTL;
	if(setsockopt(sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)))
		THROW<PosixError>("Could not set TTL to 1");

	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	if(bind(sock, (sockaddr *) &addr, sizeof(addr)))
		THROW<PosixError>("Could not bind to 0.0.0.0:", port);
}

Socket::~Socket(void)
{
	if(self->sock)
	{
		if(::close(self->sock))
		{
			std::cerr << "Failed to close socket!" << std::endl;
		}
		
		self->sock = 0;
	}
}

void Socket::join(const std::string& host)
{
	if(!self->sock)
		THROW<InvalidStateError>("Socket closed!");

	ip_mreq mreq;

	auto mca = inet_addr(host.data());

	if(mca == MXT_INVALID_ADDR)
		THROW<PosixError>("Could not resolve host ", host);

	mreq.imr_multiaddr.s_addr = mca;
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	if(setsockopt(self->sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)))
		THROW<PosixError>("Could not join ", host);
}

void Socket::listen(callback_fn f) const
{
	if(!self->sock)
		THROW<InvalidStateError>("Reading from closed socket!");

	try
	{
		while(true)
		{
			f(receive());
		}
	}
	catch(const InvalidStateError& e)
	{
		if(self->sock)
			throw e;
	}
	catch(const InterruptedError& e)
	{
		if(self->sock)
			throw e;
	}
}

Socket::buffer_t Socket::receive(void) const
{
	auto& sock(self->sock);
	auto& addr(self->addr);
	socklen_t alen = sizeof(addr);
	byte_t buf[MXT_BUFSIZE];

	if(!sock)
		THROW<InvalidStateError>("Reading from closed socket!");

	while(true)
	{
		timeval timeout = MXT_TIMEOUT;
		fd_set read_set;
		FD_ZERO(&read_set);
		FD_SET(sock, &read_set);

		if(select(sock + 1, &read_set, nullptr, nullptr, &timeout) >= 0)
		{
			if(!sock)
			{
				THROW<InterruptedError>();
			}
			else if(FD_ISSET(sock, &read_set))
			{
				int c = recvfrom(sock, buf, sizeof(buf), 0, (sockaddr *) &addr, &alen);

				if(c <= 0)
				{
					THROW<PosixError>("Receive failure");
				}
				else
				{
					return buffer_t{&buf[0], &buf[c]};
				}
			}
		}
		else
		{
			THROW<PosixError>("Select failed");
		}
	}
}

void Socket::send(const std::string& host, uint16_t port, const void *vp, size_t n)
{
	auto& addr(self->addr);
	auto& last(self->last);

	if(!self->sock)
		THROW<InvalidStateError>("Sending with closed socket!");

	if(last.host != host || last.port != port)
	{
		addr.sin_addr.s_addr = inet_addr(host.data());
		addr.sin_port = htons(port);

		last.host = host;
		last.port = port;
	}

	if(((int) n) != sendto(self->sock, vp, n, 0, (sockaddr *) &addr, sizeof(addr)))
		THROW<PosixError>("Failure sending ", n, " bytes");
}

bool Socket::open(void) const
{
	return self->sock;
}

}

