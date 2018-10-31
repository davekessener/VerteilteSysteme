#include <iostream>
#include <vector>

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random.hpp>

#include <caf/all.hpp>
#include <caf/io/all.hpp>

#include "util.h"
#include "rho.h"
#include "worker.h"

// # --------------------------------------------------------------------------

using namespace vs;

namespace distributor
{
	namespace action
	{
		using caf::atom;
		using caf::atom_constant;

		using reg = atom_constant<atom("d_reg")>;
		using unreg = atom_constant<atom("d_unreg")>;
		using process = atom_constant<atom("d_process")>;
	}

	struct result
	{
		std::vector<std::pair<uint, uint>> factors;
		uint cpu_time = 0;
		uint cycles = 0;
	};
	
	template<typename Inspector>
	typename Inspector::result_type inspect(Inspector& f, result& v)
	{
		return f(caf::meta::type_name("result"), v.factors, v.cpu_time, v.cycles);
	}
	
	using actor = caf::typed_actor<
		caf::reacts_to<action::reg, std::string, uint16_t>,
		caf::reacts_to<action::unreg, std::string, uint16_t>,
		caf::replies_to<action::process, std::string>::with<result>
	>;

	struct state
	{
		std::map<std::string, worker::actor> workers;
	};

	actor::behavior_type behavior(actor::stateful_pointer<state> self, caf::io::middleman *mm)
	{
		return {
			[=](action::reg, std::string host, uint16_t port) {
				std::string id = stringify(host, ":", port);
				auto p = mm->remote_actor<worker::actor>(host, port);
				if(p)
				{
					caf::aout(self) << "Registered worker " << id << std::endl;
					self->state.workers[id] = p.value();
				}
				else
				{
					caf::aout(self) << "Could not connect to worker " << id << ": " << p << std::endl;
				}
			},
			[=](action::unreg, std::string host, uint16_t port) {
				std::string id = stringify(host, ":", port);
				self->state.workers[id] = nullptr;
			},
			[=](action::process, std::string n) {
				uint a = 1;
				for(const auto& e : self->state.workers)
				{
					self->request(e.second, caf::infinite, worker::action::process::value, n, a)
						.then([=](std::string f) {
							caf::aout(self) << "Found " << f << std::endl;

							for(const auto& e : self->state.workers)
							{
								self->send(e.second, worker::action::abort::value);
							}
					});
				}
				return result{};
			}
		};
	}
}

// # --------------------------------------------------------------------------

using namespace caf;

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
		add_message_type<distributor::result>("result"),
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
		auto p = mm.publish(sys.spawn(worker::behavior), cfg.port);

		if(p)
		{
			std::cout << "Worker running on port " << *p << std::endl;
		}
		else
		{
			std::cerr << "Failed to create worker: " << sys.render(p.error()) << std::endl;
		}
	}
	else
	{
		scoped_actor self{sys};
		auto d = sys.spawn(distributor::behavior, &mm);

		self->send(d, distributor::action::reg::value, cfg.host, cfg.port);
		self->request(d, infinite, distributor::action::process::value, cfg.value).receive(
			[&](distributor::result r) {
			},
			[&](error& e) {
			}
		);
	}
}

CAF_MAIN(io::middleman);

