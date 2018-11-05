#include "manager.h"

#define MXT_RETRY_P (std::chrono::seconds(3))

namespace vs {

namespace manager {

struct meta_worker
{
	worker::actor w;
	uint16_t port;
	uint id;

	meta_worker( ) { }
	meta_worker(const meta_worker&) = default;
	meta_worker(const worker::actor& w, uint16_t p, uint id)
		: w(w), port(p), id(id)
	{ }
	meta_worker& operator=(const meta_worker&) = default;
};

struct state
{
	std::string host;
	std::vector<meta_worker> workers;
	caf::io::middleman *mm = nullptr;
	uint size = 0;
	uint nextID = 0;
	struct
	{
		distributor::actor handle;
		std::string host;
		uint16_t port = 0;
	} dis;
};

namespace action
{
	using caf::atom;
	using caf::atom_constant;

	using init = atom_constant<atom("m_init")>;
	using set_size = atom_constant<atom("m_size")>;
	using set_dis = atom_constant<atom("m_dis")>;
	using reconnect = atom_constant<atom("m_recon")>;
	using shutdown = atom_constant<atom("m_shutdown")>;
	using kill = atom_constant<atom("m_kill")>;
}

using actor = caf::typed_actor<
	caf::reacts_to<action::init, std::string>,
	caf::reacts_to<action::set_size, uint>,
	caf::reacts_to<action::set_dis, std::string, uint16_t>,
	caf::reacts_to<action::reconnect>,
	caf::reacts_to<action::shutdown>,
	caf::reacts_to<action::kill>
>;
	
actor::behavior_type behavior(actor::stateful_pointer<state> self)
{
	return {
		[self](action::init, const std::string& h) {
			self->state.host = h;
			self->state.mm = &self->system().middleman();

			self->set_down_handler([self](caf::down_msg& msg) {
				if(self->state.dis.handle && msg.source == self->state.dis.handle.address())
				{
					aout(self) << "Distributor " << self->state.dis.host << ":" << self->state.dis.port << " terminated!" << std::endl;

					self->state.dis.handle = nullptr;
					self->send(self, action::set_dis::value, self->state.dis.host, self->state.dis.port);
				}
				else for(const auto& w : self->state.workers)
				{
					if(msg.source == w.w.address())
					{
						aout(self) << "Worker " << w.id << " terminated!" << std::endl;
					}
				}
			});

			aout(self) << "Manager started" << std::endl;
		},
		[self](action::shutdown) {
			aout(self) << "Shutting manager down" << std::endl;

			self->send(self, action::set_size::value, uint{0});
			self->send(self, action::kill::value);
		},
		[self](action::kill) {
			self->quit();
		},
		[self](action::set_size, uint s) {
			aout(self) << "Manager-pool resized to " << s << std::endl;

			self->state.size = s;

			caf::actor_system& sys(self->system());
			caf::io::middleman& mm(*self->state.mm);

			while(self->state.workers.size() < s)
			{
				uint id = self->state.nextID++;
				auto w = sys.spawn(worker::behavior, id);
				auto p = mm.publish(w, 0);

				if(!p)
				{
					aout(self) << "ERR: Failed to publish worker: " << sys.render(p.error()) << std::endl;
				}
				else
				{
					aout(self) << "Worker " << id << " running on port " << *p << std::endl;

					self->state.workers.emplace_back(w, *p, id);
					self->monitor(w);

					if(self->state.dis.handle)
					{
						self->send(self->state.dis.handle, distributor::action::reg::value, self->state.host, *p);
					}
				}
			}

			while(self->state.workers.size() > s)
			{
				auto& w(self->state.workers.back());

				aout(self) << "Shutting down worker " << w.id << std::endl;

				mm.unpublish(w.w, w.port);

//				self->send(w.w, worker::action::kill::value);
				self->send_exit(w.w, caf::exit_reason::user_shutdown);

				self->state.workers.pop_back();
			}
		},
		[self](action::set_dis, const std::string& host, uint16_t port) {
			aout(self) << "Setting distributor " << host << ":" << port << std::endl;

			if(self->state.dis.handle)
			{
				aout(self) << "Unregistering with " << self->state.dis.host << ":" << self->state.dis.port << std::endl;

				self->demonitor(self->state.dis.handle.address());

				for(const auto& w : self->state.workers)
				{
					self->send(self->state.dis.handle, distributor::action::unreg::value, self->state.host, w.port);
				}

				self->state.dis.handle = nullptr;
			}

			self->state.dis.host = host;
			self->state.dis.port = port;

			auto r = self->system().middleman().remote_actor<distributor::actor>(host, port);

			if(r)
			{
				self->state.dis.handle = r.value();

				for(const auto& w : self->state.workers)
				{
					self->send(self->state.dis.handle, distributor::action::reg::value, self->state.host, w.port);
				}

				self->monitor(self->state.dis.handle);
			}
			else
			{
				aout(self) << "Failed to connect!" << std::endl;

				self->delayed_send(self, MXT_RETRY_P, action::reconnect::value);
			}
		},
		[self](action::reconnect) {
			if(!self->state.dis.handle)
			{
				self->send(self, action::set_dis::value, self->state.dis.host, self->state.dis.port);
			}
		}
	};
}

}

// # --------------------------------------------------------------------------

struct Manager::Impl
{
	caf::actor_system *sys;
	manager::actor actor;
};

Manager::Manager(const std::string& h, caf::actor_system& sys, uint s)
	: pImpl(new Impl)
{
	caf::scoped_actor self{sys};

	pImpl->sys = &sys;
	pImpl->actor = sys.spawn<caf::detached>(manager::behavior);

	self->send(pImpl->actor, manager::action::init::value, h);
	self->send(pImpl->actor, manager::action::set_size::value, s);
}

Manager::~Manager(void)
{
	caf::scoped_actor self{*pImpl->sys};

	self->send(pImpl->actor, manager::action::shutdown::value);

	delete pImpl;
	pImpl = nullptr;
}

void Manager::setDistributor(const std::string& host, uint16_t port)
{
	caf::scoped_actor self(*pImpl->sys);

	self->send(pImpl->actor, manager::action::set_dis::value, host, port);
}

void Manager::setPoolSize(uint s)
{
	caf::scoped_actor self{*pImpl->sys};

	self->send(pImpl->actor, manager::action::set_size::value, s);
}

}

