#include "avatar.h"

namespace vs { namespace avatar {

actor::behavior_type behavior(actor::stateful_pointer<state> self)
{
	return {
		[self](action::create, uint16_t port) {
			self->state.handle = self->system().spawn(distributor::behavior, &self->system().middleman());
			auto p = self->system().middleman().publish(self->state.handle, port, nullptr, true);

			if(p)
			{
				self->state.port = *p;
			}

			return p;
		},
		[self](action::calculate, const std::string& n) {
			self->request(self->state.handle, caf::infinite, distributor::action::process::value, n).then(
				[self, n](const distributor::result& r) {
					uint512_t cmp = 1;

					aout(self) << n << ": 1";
					for(const auto& f : r.factors)
					{
						for(uint n = f.second ; n-- ;)
						{
							cmp *= uint512_t{f.first};

							aout(self) << " * " << f.first;
						}
					}
					aout(self) << std::endl;

					aout(self) << (cmp == uint512_t{n} ? "SUCCESS" : "FAILURE") << std::endl;
					aout(self) << "It took " << r.cycles << " cycles." << std::endl;
					aout(self) << "CPU ran for " << r.time.cpu << "s; total time: " << r.time.total << "s" << std::endl;
				},
				[&](const caf::error& e) {
					aout(self) << "some error: " << self->system().render(e) << std::endl;
				}
			);
		},
		[self](action::kill) {
			if(self->state.handle)
			{
				self->system().middleman().unpublish(self->state.handle, self->state.port);
				self->send(self->state.handle, distributor::action::kill::value);
			}

			self->quit();
		}
	};
}

}}

