#ifndef VS_AVATAR_H
#define VS_AVATAR_H

#include "util.h"
#include "distributor.h"

namespace vs
{
	namespace avatar
	{
		namespace action
		{
			using caf::atom;
			using caf::atom_constant;

			using create = atom_constant<atom("a_create")>;
			using calculate = atom_constant<atom("a_calc")>;
			using kill = atom_constant<atom("a_kill")>;
		}

		using actor = caf::typed_actor<
			caf::replies_to<action::create, uint16_t>::with<uint16_t>,
			caf::reacts_to<action::calculate, std::string>,
			caf::reacts_to<action::kill>
		>;

		struct state
		{
			distributor::actor handle;
			uint16_t port;
		};

		actor::behavior_type behavior(actor::stateful_pointer<state>);
	}
}

#endif

