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
			using abort = atom_constant<atom("w_abort")>;
		}

		struct result
		{
			std::string factor, source;
			double cpu_time = 0;
			uint cycles = 0;
			uint tries = 0;
		};

		using actor = caf::typed_actor<
			caf::replies_to<action::process, std::string, uint, uint>::with<result>,
			caf::reacts_to<action::abort>
		>;

		class State
		{
			typedef boost::random::uniform_int_distribution<uint512_t> d_t;

			public:
				void set(uint512_t, uint, uint);
				void step( );
				result get( ) const;

				bool done( ) const { return mFact.done(); }
				void update(uint64_t t) { mTime += t; }

			private:
				void abort( );
				void reset( );

			private:
				boost::random::mt19937 mMT;
				d_t mRNG;
				uint512_t mNumber;
				uint mA;
				uint512_t mCount, mLeft;
				uint mCycles, mTries;
				uint64_t mTime;
				RhoFactorizer<uint512_t> mFact;
		};

		actor::behavior_type behavior(actor::stateful_pointer<State>, uint);

		template<typename Inspector>
		typename Inspector::result_type inspect(Inspector& f, result& r)
		{
			return f(caf::meta::type_name("w_result"), r.factor, r.source, r.cpu_time, r.cycles, r.tries);
		}
	}
}

#endif

