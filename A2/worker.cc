#include "worker.h"

namespace vs { namespace worker {

actor::behavior_type behavior(actor::stateful_pointer<state> self)
{
	return {
		[=](action::process, std::string n, uint a) {
			boost::random::mt19937 mt;
			boost::random::uniform_int_distribution<uint512_t> rng;

			self->state.fact = { uint512_t{n}, rng(mt), a };

			return self->delegate(actor{self}, action::resume::value);
		},
		[=](action::resume) -> caf::result<std::string> {
			auto *ft = &self->state.fact;

			while(!ft->done())
			{
				ft->step();

				if(!self->mailbox().empty())
				{
					return self->delegate(actor{self}, action::resume::value);
				}

				if(!self->state.running)
				{
					return "";
				}
			}

			return stringify(ft->get());
		},
		[=](action::abort) {
			self->state.running = false;
		}
	};
}

}}

