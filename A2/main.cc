#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>

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
		using step = atom_constant<atom("d_step")>;
	}

	struct result
	{
		std::vector<std::pair<std::string, uint>> factors;
		uint cpu_time = 0;
		uint cycles = 0;
	};
	
	template<typename Inspector>
	typename Inspector::result_type inspect(Inspector& f, result& v)
	{
		return f(caf::meta::type_name("d_result"), v.factors, v.cpu_time, v.cycles);
	}
	
	using actor = caf::typed_actor<
		caf::reacts_to<action::reg, std::string, uint16_t>,
		caf::reacts_to<action::unreg, std::string, uint16_t>,
		caf::replies_to<action::process, std::string>::with<result>,
		caf::reacts_to<action::step, std::string, std::string>
	>;

	struct state
	{
		uint a = 1;
		std::map<std::string, worker::actor> workers;
		std::deque<uint512_t> open;
		result r;
		caf::response_promise promise;
	};

	actor::behavior_type behavior(actor::stateful_pointer<state> self, caf::io::middleman *mm)
	{
		return {
			[=](action::reg, const std::string& host, uint16_t port) {
				std::string id = stringify(host, ":", port);
				auto p = mm->remote_actor<worker::actor>(host, port);

				if(p)
				{
					self->state.workers[id] = p.value();

					caf::aout(self) << "Registered worker " << id << std::endl;
				}
				else
				{
					caf::aout(self) << "Could not connect to worker " << id << ": " << p << std::endl;
				}
			},
			[=](action::unreg, const std::string& host, uint16_t port) {
				std::string id = stringify(host, ":", port);
				self->state.workers[id] = nullptr;
				aout(self) << "Removed worker " << id << std::endl;
			},
			[=](action::process, const std::string& n) -> caf::result<result> {
				self->state.a = 1;
				self->state.open.clear();
				self->state.r = result{};
				self->state.promise = self->make_response_promise<result>();
				self->send(self, action::step::value, n, "");

				caf::aout(self) << "Received request for factorization of " << n << std::endl;

				return self->state.promise;
			},
			[=](action::step, const std::string& n, const std::string& src) {
				if(!self->state.open.empty() && self->state.open.front() != uint512_t{src})
					return;

				uint512_t v(n);

				while(isPrime(v) || v == 1)
				{
					if(self->state.open.empty())
					{
						for(const auto& e : self->state.workers)
						{
							self->send(e.second, worker::action::abort::value);
						}

						if(v != 1)
						{
							self->state.r.factors.push_back(std::make_pair(stringify(v), 1));
						}

						std::sort(self->state.r.factors.begin(), self->state.r.factors.end(),
							[](const std::pair<std::string, uint>& a, const std::pair<std::string, uint>& b) {
								return uint512_t{a.first} < uint512_t{b.first};
						});

						self->state.promise.deliver(self->state.r);

						return;
					}
					else if(v == 1)
					{
						v = self->state.open.front();
						self->state.open.pop_front();
					}
					else
					{
						uint c = 0;

						for(auto& o : self->state.open)
						{
							c = divideAll(o, v);
						}

						aout(self) << "Found prime " << stringify(v) << "^" << c << std::endl;

						self->state.r.factors.push_back(std::make_pair(stringify(v), c));

						v = 1;
					}
				}

				self->state.open.push_front(v);

				for(const auto& e : self->state.workers)
				{
					aout(self) << "Instructing worker " << e.first << " to compute factors of " << stringify(v) << std::endl;

					self->request(e.second, caf::infinite,
						worker::action::process::value, stringify(v), self->state.a, 1000u)
					.then([self, id = e.first](const worker::result& r) {
						self->state.r.cycles += r.cycles;
						self->state.r.cpu_time += r.cpu_time;

						aout(self) << "Received answer from " << id << ": " << r.factor << std::endl;

						if(!self->state.open.empty() && r.factor != "")
						{
							self->send(self, action::step::value, r.factor, r.source);
						}
					});

					self->state.a += 2;
				}
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
		add_message_type<distributor::result>("d_result"),
		add_message_type<worker::result>("w_result"),
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
		for(uint port = 0 ; port < cfg.port ; ++port)
		{
			auto p = mm.publish(sys.spawn(worker::behavior, 8000 + port), 8000 + port);
	
			if(p)
			{
				std::cout << "Worker running on port " << *p << std::endl;
			}
			else
			{
				std::cerr << "Failed to create worker: " << sys.render(p.error()) << std::endl;
				return;
			}
		}
	}
	else
	{
		scoped_actor self{sys};
		auto d = sys.spawn(distributor::behavior, &mm);

		for(uint i = 0 ; i < cfg.port ; ++i)
		{
			self->send(d, distributor::action::reg::value, std::string{"localhost"}, (uint16_t)(8000 + i));
		}
//		self->send(d, distributor::action::reg::value, cfg.host, cfg.port);
		self->request(d, infinite, distributor::action::process::value, cfg.value).receive(
			[&](distributor::result r) {
				uint512_t cmp = 1;

				aout(self) << cfg.value << ": 1";
				for(const auto& f : r.factors)
				{
					for(uint n = f.second ; n-- ;)
					{
						cmp *= uint512_t{f.first};

						aout(self) << " * " << f.first;
					}
				}
				aout(self) << std::endl;

				aout(self) << (cmp == uint512_t{cfg.value} ? "SUCCESS" : "FAILURE") << std::endl;
				aout(self) << "It took " << r.cycles << " cycles." << std::endl;
			},
			[&](error& e) {
				aout(self) << "some error: " << sys.render(e) << std::endl;
			}
		);
	}
}

CAF_MAIN(io::middleman);

