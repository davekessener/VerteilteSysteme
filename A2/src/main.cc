#include <iostream>

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random.hpp>
#include <boost/lexical_cast.hpp>

#include <caf/all.hpp>
#include <caf/io/all.hpp>

#include "util.h"
#include "rho.h"
#include "worker.h"
#include "distributor.h"
#include "manager.h"
#include "avatar.h"

// # --------------------------------------------------------------------------

using namespace vs;
using namespace caf;

std::ostream& operator<<(std::ostream& os, const expected<message>& msg)
{
	return os << to_string(msg);
}

struct config : actor_system_config
{
	std::string value = "1";
	std::string host = "localhost";
	std::string dist = "localhost";
	uint16_t port = 0;
	bool server = false;

	config( )
	{
		add_message_type<distributor::result>("d_result");
		add_message_type<worker::result>("w_result");
//		add_message_type<worker::actor>("worker_actor");
		opt_group{custom_options_, "global"}
			.add(value, "value,V", "Value to be factorized")
			.add(host, "host,H", "Server hostname")
			.add(dist, "distributor,d", "Remote distributor")
			.add(port, "port,p", "Port")
			.add(server, "server,s", "Run server");
	}
};

void caf_main(actor_system& sys, const config& cfg)
{
	std::cout << "VSP/2 - Lab 2" << std::endl;

	if(cfg.server)
	{
		Manager manager(cfg.host, sys);

		manager.setDistributor(cfg.dist, cfg.port);

		for(std::string line ; std::getline(std::cin, line) ;)
		{
			if(line.empty()) continue;

			if(line[0] == 'q') break;
			
			if(line[0] == 'r')
			{
				manager.setPoolSize(boost::lexical_cast<int>(std::string{line.data() + 1}));
			}
		}
	}
	else
	{
		scoped_actor self{sys};
		auto a = sys.spawn(avatar::behavior);
		uint16_t p = 0;

		self->request(a, infinite, avatar::action::create::value, cfg.port).receive(
			[&](uint16_t port) {
				p = port;
			},
			[&](const error& e) {
				aout(self) << "ERR: " << e << std::endl;
			}
		);

		if(p)
		{
			std::cout << "Distributor running on port " << p << std::endl;

			for(std::string line ; std::getline(std::cin, line) ;)
			{
				if(line.empty()) continue;
				if(line[0] == 'q') break;
				
				self->send(a, avatar::action::calculate::value, line);
			}
		}

		self->send(a, avatar::action::kill::value);
	}

	std::cout << "Goodbye." << std::endl;
}

CAF_MAIN(io::middleman);

