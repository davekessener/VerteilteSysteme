#ifndef VS_DISTRIBUTOR_H
#define VS_DISTRIBUTOR_H

#include <map>
#include <deque>
#include <chrono>

#include "util.h"
#include "worker.h"

namespace vs
{
	namespace distributor
	{
		namespace action
		{
			using caf::atom;
			using caf::atom_constant;
	
			using reg = atom_constant<atom("d_reg")>;
			using process = atom_constant<atom("d_process")>;
			using start = atom_constant<atom("d_start")>;
			using reply = atom_constant<atom("d_reply")>;
			using request = atom_constant<atom("d_req")>;
			using next = atom_constant<atom("d_next")>;
			using eval = atom_constant<atom("d_eval")>;
		}
	
		struct result
		{
			std::vector<std::pair<std::string, uint>> factors;
			struct
			{
				double cpu = 0;
				double total = 0;
			} time;
			uint cycles = 0;
		};
		
		using actor = caf::typed_actor<
			caf::reacts_to<action::reg, worker::actor>,
			caf::replies_to<action::process, std::string>::with<result>,
			caf::reacts_to<action::start>,
			caf::reacts_to<action::reply>,
			caf::reacts_to<action::request, worker::actor, std::string, uint>,
			caf::reacts_to<action::next>,
			caf::reacts_to<action::eval, std::string, std::string>,
			caf::reacts_to<worker::result>
		>;
	
		struct state
		{
			using clock_type = std::chrono::high_resolution_clock;

			uint a = 1;
			uint pending = 0;
			std::vector<worker::actor> workers;
			std::vector<uint512_t> open;
			result r;
			caf::response_promise promise;
			clock_type::time_point start;
			std::deque<std::pair<std::string, caf::response_promise>> backlog;
		};

		actor::behavior_type behavior(actor::stateful_pointer<state>);
		
		template<typename Inspector>
		typename Inspector::result_type inspect(Inspector& f, result& v)
		{
			return f(caf::meta::type_name("d_result"), v.factors, v.time.cpu, v.time.total, v.cycles);
		}
	
	}
}

#endif

