#include <algorithm>

#include "distributor.h"

#include "rho.h"

#define MXT_L uint{1000}

namespace vs { namespace distributor {

namespace
{
	typedef caf::atom_constant<caf::atom("system")> system_atom;

	template<typename T>
	inline void incA(T& a)
	{
		a += 2;
	}
}

actor::behavior_type behavior(actor::stateful_pointer<state> self)
{
	self->set_down_handler([self](caf::down_msg& msg) {
		auto i = std::find_if(self->state.workers.begin(), self->state.workers.end(),
			[a = msg.source](const worker::actor& w) { return w.address() == a; });

		if(i != self->state.workers.end())
		{
			caf::aout(self) << "Lost worker " << *i << std::endl;

			self->state.workers.erase(i);
		}
	});

	return {
		[=](action::reg, worker::actor w) {
			self->state.workers.push_back(w);

			caf::aout(self) << "Registered new worker " << w << std::endl;

			self->monitor(w);
			if(!self->state.open.empty())
			{
				self->send(self, action::request::value, w, stringify(self->state.open.front()), self->state.a);
				incA(self->state.a);
			}
		},
		[=](action::process, const std::string& n) -> caf::result<result> {
			auto p = self->make_response_promise<result>();
			
			self->state.backlog.emplace_back(n, p);

			if(self->state.open.empty())
			{
				self->send(self, action::start::value);
			}

			caf::aout(self) << "Received request for factorization of " << n << std::endl;
			
			return p;
		},
		[=](action::start) {
			if(self->state.backlog.empty())
			{
				aout(self) << "ERR: tried to start without a request!" << std::endl;
			}
			else
			{
				auto& req(self->state.backlog.front());
				uint512_t v{req.first};

				self->state.a = 1;
				self->state.pending = 0;
				self->state.open.clear();
				self->state.r = result{};
				self->state.promise = req.second;
				self->state.start = state::clock_type::now();

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

				aout(self) << "Starting to process " << req.first << std::endl;

				self->state.open.push_back(v);
				self->send(self, action::next::value);

				self->state.backlog.pop_front();
			}
		},
		[=](action::reply) {
			if(!self->state.pending)
			{
				self->state.promise.deliver(self->state.r);

				aout(self) << "Done processing!" << std::endl;

				if(!self->state.backlog.empty())
				{
					self->send(self, action::start::value);
				}
			}
		},
		[=](action::next) {
			if(self->state.open.empty())
				return;

			for(auto& w : self->state.workers)
			{
				std::string v(stringify(self->state.open.back()));
				self->send(self, action::request::value, w, v, self->state.a);
				incA(self->state.a);
			}
		},
		[=](action::eval, const std::string& n, const std::string& src) {
			if(self->state.open.empty())
				return;
			if(self->state.open.back() != uint512_t{src})
				return;

			uint512_t v(n);

			while(isPrime(v) || v == 1)
			{
				if(self->state.open.empty())
				{
					self->state.r.time.total = std::chrono::duration_cast<std::chrono::microseconds>(state::clock_type::now() - self->state.start).count() / 1000000.0;

					for(auto& w : self->state.workers)
					{
						self->send(w, worker::action::abort::value);
					}

					if(v != 1)
					{
						self->state.r.factors.emplace_back(stringify(v), 1);
					}

					std::sort(self->state.r.factors.begin(), self->state.r.factors.end(),
						[](const std::pair<std::string, uint>& a, const std::pair<std::string, uint>& b) {
							return uint512_t{a.first} < uint512_t{b.first};
					});

					self->send(self, action::reply::value);

					return;
				}
				else if(v == 1)
				{
					v = self->state.open.back();
					self->state.open.pop_back();
				}
				else
				{
					uint c = 0;

					for(auto& o : self->state.open)
					{
						c = divideAll(o, v);
					}

					aout(self) << "Found prime " << stringify(v) << "^" << c << std::endl;

					self->state.r.factors.emplace_back(stringify(v), c);

					v = 1;
				}
			}

			self->state.open.push_back(v);
			self->send(self, action::next::value);
		},
		[=](action::request, worker::actor w, const std::string& v, uint a) {
			if(self->state.open.empty())
				return;
			
			auto idx = std::find(self->state.workers.begin(), self->state.workers.end(), w);

			if(idx != self->state.workers.end() && uint512_t{v} == self->state.open.back())
			{
				++self->state.pending;

				self->send(w, worker::action::process::value, v, a, MXT_L);
			}
		},
		[=](const worker::result& r) {
			--self->state.pending;

			self->state.r.cycles += r.cycles;
			self->state.r.time.cpu += r.cpu_time;

			if(self->state.open.empty())
			{
				if(!self->state.pending)
				{
					self->send(self, action::reply::value);
				}
			}
			else if(r.factor != "")
			{
				self->send(self, action::eval::value, r.factor, r.source);
			}
		}
	};
}

}}

