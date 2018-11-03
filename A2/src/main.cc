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
	uint16_t port = 0;
	bool server = false;

	config( )
	{
		add_message_type<distributor::result>("d_result"),
		add_message_type<worker::result>("w_result"),
		opt_group{custom_options_, "global"}
			.add(value, "value,V", "Value to be factorized")
			.add(host, "host,H", "Server hostname")
			.add(port, "port,p", "Port")
			.add(server, "server,s", "Run server");
	}
};

void caf_main(actor_system& sys, const config& cfg)
{
	std::cout << "VSP/2 - Lab 2" << std::endl;

	auto& mm = sys.middleman();

	if(cfg.server)
	{
		Manager manager(cfg.host, sys);

		manager.setDistributor(cfg.host, cfg.port);

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
		auto d = sys.spawn(distributor::behavior, &mm);

		auto p = mm.publish(d, cfg.port, nullptr, true);

		// TODO "specter" && time measurement && why do workers crash?

		if(!p)
		{
			std::cerr << "Could not publish distributor: " << sys.render(p.error()) << std::endl;
		}
		else
		{
			std::cout << "Distributor running on port " << *p << std::endl;

			for(std::string line ; std::getline(std::cin, line) ;)
			{
				if(line.empty()) continue;
				if(line[0] == 'q') break;
				
				self->request(d, infinite, distributor::action::process::value, line).receive(
					[&](const distributor::result& r) {
						uint512_t cmp = 1;

						aout(self) << cfg.value << ": 1";
						for(const auto& f : r.factors)
						{
							for(uint n = f.second ; n-- ;)
							{
								cmp *= uint512_t{f.first};

								aout(self) << " * " << f.first;
							}
						}
						aout(self) << std::endl;

						aout(self) << (cmp == uint512_t{line} ? "SUCCESS" : "FAILURE") << std::endl;
						aout(self) << "It took " << r.cycles << " cycles." << std::endl;
					},
					[&](error& e) {
						aout(self) << "some error: " << sys.render(e) << std::endl;
					}
				);

				std::cout << "HERE" << std::endl;
			}

			mm.unpublish(d, cfg.port);
		}

		self->send(d, distributor::action::kill::value);

		aout(self) << "Goodbye." << std::endl;
	}
}

CAF_MAIN(io::middleman);

