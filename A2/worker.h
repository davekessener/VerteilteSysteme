#ifndef VS_WORKER_H
#define VS_WORKER_H

#include "util.h"
#include "rho.h"

namespace vs
{
	namespace worker
	{
		namespace action
		{
			using caf::atom;
			using caf::atom_constant;

			using process = atom_constant<atom("w_process")>;
			using resume = atom_constant<atom("w_resume")>;
			using abort = atom_constant<atom("w_abort")>;
		}

		using actor = caf::typed_actor<
			caf::replies_to<action::process, std::string, uint>::with<std::string>,
			caf::replies_to<action::resume>::with<std::string>,
			caf::reacts_to<action::abort>
		>;

		struct state
		{
			bool running = true;
			RhoFactorizer<uint512_t> fact;
		};

		actor::behavior_type behavior(actor::stateful_pointer<state>);
	}
}

#endif

