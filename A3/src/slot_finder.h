#ifndef VS_SLOTFINDER_H
#define VS_SLOTFINDER_H

#include <random>

#include "util.h"
#include "schedule.h"
#include "error.h"

namespace vs
{
	template
	<
		size_t N,
		size_t C = 3
	>
	class SlotFinder
	{
		public:
		DEFINE_EXCEPTION(NoEmptySlotError);

		public:
			SlotFinder( );
			uint operator()(const Schedule<N>&);

		private:
			std::random_device mSeed;
			std::mt19937 mGen;
			std::uniform_int_distribution<uint> mRNG;
	};

	template<size_t N, size_t C>
	SlotFinder<N, C>::SlotFinder(void)
		: mGen(mSeed())
		, mRNG(0, N-1)
	{
	}

	template<size_t N, size_t C>
	uint SlotFinder<N, C>::operator()(const Schedule<N>& schedule)
	{
		uint next_slot = 0;

		for(uint i = 0 ; i < C ; ++i)
		{
			next_slot = mRNG(mGen);

			if(!schedule.check(next_slot))
				return next_slot;
		}

		std::vector<uint> pot;

		for(uint i = 0 ; i < N ; ++i)
		{
			if(!schedule.check(i))
			{
				pot.push_back(i);
			}
		}

		if(pot.empty())
			THROW<NoEmptySlotError>();

		return pot[next_slot % pot.size()];
	}
}

#endif

