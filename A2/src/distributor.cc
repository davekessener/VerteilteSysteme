#include <algorithm>

#include "distributor.h"

#include "rho.h"

namespace vs { namespace distributor {

actor::behavior_type behavior(actor::stateful_pointer<state> self, caf::io::middleman *mm)
{
	return {
		[=](action::kill) {
			self->quit();
		},
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

			uint512_t v{n};

			uint c = 0;
			while(!(v & 1))
			{
				v >>= 1;
				++c;
			}
			if(c)
			{
				self->state.r.factors.emplace_back("2", c);
			}

			self->send(self, action::step::value, stringify(v), "");

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

}}

