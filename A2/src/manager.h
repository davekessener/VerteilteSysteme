#ifndef VS_MANAGER_H
#define VS_MANAGER_H

#include "util.h"
#include "worker.h"
#include "distributor.h"

namespace vs
{
	class Manager
	{
		typedef std::pair<worker::actor, uint16_t> worker_t;

		public:
			Manager(caf::actor_system&, uint = 0, uint = 1);
			~Manager( );
			void setDistributor(const std::string&, uint16_t);
			void setPoolSize(uint);

		private:
			struct Impl;

			Impl *pImpl;
	};
}

#endif

