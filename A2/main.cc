#include <iostream>
#include <sstream>
#include <stdint.h>

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random.hpp>

#include <caf/all.hpp>
#include <caf/io/all.hpp>

#include "rho.h"


namespace impl
{
	template<typename S>
	void stringify(S&)
	{
	}

	template<typename S, typename T, typename ... TT>
	void stringify(S& ss, const T& o, const TT& ... a)
	{
		ss << o;

		stringify(ss, a...);
	}
}

template<typename ... T>
std::string stringify(const T& ... a)
{
	std::stringstream ss;

	impl::stringify(ss, a...);

	return ss.str();
}


using namespace caf;

using boost::multiprecision::uint512_t;
typedef unsigned uint;

using w_process_a = atom_constant<atom("w_process")>;
using w_resume_a = atom_constant<atom("w_resume")>;
using w_abort_a = atom_constant<atom("w_abort")>;


using worker = typed_actor<
	replies_to<w_process_a, std::string, uint>::with<std::string>,
	replies_to<w_resume_a>::with<std::string>,
	reacts_to<w_abort_a>
>;

struct worker_state
{
	bool running = true;
	vs::RhoFactorizer<uint512_t> fact;
};

worker::behavior_type worker_actor(worker::stateful_pointer<worker_state> self)
{
	return {
		[=](w_process_a, std::string n, uint a) {
			boost::random::mt19937 mt;
			boost::random::uniform_int_distribution<uint512_t> rng;

			self->state.fact = { uint512_t{n}, rng(mt), a };

			return self->delegate(worker{self}, w_resume_a::value);
		},
		[=](w_resume_a) -> result<std::string> {
			auto *ft = &self->state.fact;

			while(!ft->done())
			{
				ft->step();

				if(!self->mailbox().empty())
				{
					return self->delegate(worker{self}, w_resume_a::value);
				}

				if(!self->state.running)
				{
					return "";
				}
			}

			return stringify(ft->get());
		},
		[=](w_abort_a) {
			self->state.running = false;
		}
	};
}

std::ostream& operator<<(std::ostream& os, const expected<message>& msg)
{
	return os << to_string(msg);
}

struct config : actor_system_config
{
	std::string value = "1";
	std::string host = "localhost";
	uint16_t port = 0;
	bool server = false;

	config( ) {
		opt_group{custom_options_, "global"}
			.add(value, "value,V", "Value to be factorized")
			.add(host, "host,H", "Server hostname")
			.add(port, "port,p", "Port")
			.add(server, "server,s", "Run server");
	}
};

void caf_main(actor_system& sys, const config& cfg)
{
	std::cout << "VSP/2 - Lab 2" << std::endl;

	auto& mm = sys.middleman();

	if(cfg.server)
	{
		auto p = mm.publish(sys.spawn(worker_actor), cfg.port);

		if(p)
		{
			std::cout << "Worker started on port " << *p << std::endl;
		}
		else
		{
			std::cerr << "Failed to create worker: " << sys.render(p.error()) << std::endl;
		}
	}
	else
	{
		auto a = mm.remote_actor<worker>(cfg.host, cfg.port);

		if(a)
		{
			using namespace std::chrono_literals;

			scoped_actor self{sys};

			aout(self) << "Requesting a factor of " << cfg.value << std::endl;

			self->send(*a, w_process_a::value, cfg.value, 3u);

			std::this_thread::sleep_for(1s);

			if(self->mailbox().empty())
			{
				self->send(*a, w_abort_a::value);
				aout(self) << "Timeout; aborting ..." << std::endl;
			}
			else
			{
				self->receive([&](const std::string& f) {
					aout(self) << "Received factor " << f << std::endl;
				});
			}

//			auto f = make_function_view(*a);
//			std::cout << "A pot. prime factor of " << cfg.value << ": " << f(w_process_a::value, cfg.value, 3u) << std::endl;
		}
		else
		{
			std::cerr << "Failed to connect to worker: " << sys.render(a.error()) << std::endl;
		}
	}
}

CAF_MAIN(io::middleman);

