#include <algorithm>

#include "distributor.h"

#include "rho.h"

#define MXT_WORKER_TIMEOUT std::chrono::seconds(15)
#define MXT_L uint{1000}

#define MXT_E_TIMEOUT 4

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

				if(!self->state.open.empty())
				{
					self->send(self, action::request::value, id, stringify(self->state.open.front()), self->state.a);

					incA(self->state.a);
				}
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
			self->state.pending = 0;
			self->state.open.clear();
			self->state.r = result{};
			self->state.promise = self->make_response_promise<result>();
			self->state.start = state::clock_type::now();

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

			self->state.open.push_front(v);
			self->send(self, action::next::value);

			caf::aout(self) << "Received request for factorization of " << n << std::endl;

			return self->state.promise;
		},
		[=](action::reply) {
			if(!self->state.pending)
			{
				self->state.promise.deliver(self->state.r);
			}
		},
		[=](action::next) {
			if(self->state.open.empty())
				return;

			for(const auto& e : self->state.workers)
			{
				std::string v(stringify(self->state.open.front()));

				aout(self) << "Instructing worker " << e.first << " to compute factors of " << v << std::endl;

				self->send(self, action::request::value, e.first, v, self->state.a);
				incA(self->state.a);
			}
		},
		[=](action::eval, const std::string& n, const std::string& src) {
			if(self->state.open.empty())
				return;
			if(self->state.open.front() != uint512_t{src})
				return;

			uint512_t v(n);

			while(isPrime(v) || v == 1)
			{
				if(self->state.open.empty())
				{
					self->state.r.time.total = std::chrono::duration_cast<std::chrono::microseconds>(state::clock_type::now() - self->state.start).count() / 1000000.0;

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

					self->send(self, action::reply::value);

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
			self->send(self, action::next::value);
		},
		[self](action::request, const std::string& id, const std::string& v, uint a) {
			if(self->state.open.empty())
				return;

			auto& w(self->state.workers[id]);

			if(w && uint512_t{v} == self->state.open.front())
			{
				++self->state.pending;

				self->request(w, MXT_WORKER_TIMEOUT, worker::action::process::value, v, a, MXT_L).then(
					[self, id](const worker::result& r) {
						--self->state.pending;

						self->state.r.cycles += r.cycles;
						self->state.r.time.cpu += r.cpu_time;

						aout(self) << "Received answer from " << id << ": " << r.factor << std::endl;

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
					},
					[self, id, v, a](caf::error e) {
						--self->state.pending;

						if(e.category() == system_atom::value && e.code() == MXT_E_TIMEOUT)
						{
							self->send(self, action::request::value, id, v, a);
						}
						else
						{
							aout(self) << "ERR: unexpected error: " << e << std::endl;
						}
					}
				);
			}
		}
	};
}

}}

