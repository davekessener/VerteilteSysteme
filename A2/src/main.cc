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
	std::string host = "localhost";
	std::string value = "";
	uint16_t port = 0;
	bool worker = false;
	bool dist = false;
	bool user = false;
	bool json = false;
	uint count = 1;

	config( )
	{
		add_message_type<distributor::result>("d_result");
		add_message_type<worker::result>("w_result");
		add_message_type<worker::actor>("worker_handle");
		opt_group{custom_options_, "global"}
			.add(host, "host,H", "Server hostname")
			.add(port, "port,p", "Port")
			.add(worker, "worker,w", "Starts workers")
			.add(dist, "distributor,d", "Starts distributor")
			.add(user, "user,u", "Acts as user")
			.add(count, "count,c", "Number of workers to start")
			.add(value, "value,V", "Value to be processed")
			.add(json, "json,j", "Output results in JSON format");
	}
};

void caf_main(actor_system& sys, const config& cfg)
{
	if(!cfg.json)
	{
		std::cout << "VSP/2 - Lab 2" << std::endl;
	}

	uint check = (cfg.worker ? 1 : 0) + (cfg.dist ? 1 : 0) + (cfg.user ? 1 : 0);
	auto& mm(sys.middleman());
	scoped_actor self{sys};

	if(check != 1)
	{
		std::cerr << "ERR: Invalid arguments!" << std::endl;
		std::cerr << "Selected either --worker, --distributor OR --user" << std::endl;
	}
	else if(cfg.worker)
	{
		Manager manager(sys, cfg.count);

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
	else if(cfg.dist)
	{
		auto d = sys.spawn(distributor::behavior);
		auto p = mm.publish(d, cfg.port);

		if(p)
		{
			std::cout << "Started Distributor on port " << *p << std::endl;

			for(std::string line ; std::getline(std::cin, line) ;)
			{
				if(line.empty()) continue;
				if(line[0] == 'q') break;
			}

			mm.unpublish(d, *p);
			self->send_exit(d, exit_reason::user_shutdown);

			std::cout << "Distributor shutting down ..." << std::endl;
		}
		else
		{
			std::cerr << "Failed to publish distributor: " << sys.render(p.error()) << std::endl;
		}
	}
	else if(cfg.user)
	{
		auto d = mm.remote_actor<distributor::actor>(cfg.host, cfg.port);

		if(!d)
		{
			std::cerr << "ERR: failed to connect to distributor: " << sys.render(d.error()) << std::endl;
		}
		else
		{
			auto process = [&](const std::string& v) {
				self->request(d.value(), infinite, distributor::action::process::value, v).receive(
					[&](const distributor::result& r) {
						if(cfg.json)
						{
							std::cout << "{\"factors\" : [";
							bool first = true;
							for(const auto& f : r.factors)
							{
								for(uint i = 0 ; i < f.second ; ++i)
								{
									if(!first) std::cout << ", ";
									std::cout << '"' << f.first << '"';
									first = false;
								}
							}
							std::cout << "], "
									  << "\"cycles\" : " << r.cycles << ", "
									  << "\"time\" : {\"cpu\" : " << r.time.cpu << ", "
									  << "\"total\" : " << r.time.total << "}}"
									  << std::endl;
						}
						else
						{
							std::cout << "1 ";
							for(const auto& f : r.factors)
							{
								for(uint i = 0 ; i < f.second ; ++i)
								{
									std::cout << "* " << f.first;
								}
							}
							std::cout << "\n"
									  << "Cycles: " << r.cycles << ", "
									  << "CPU-Time: " << r.time.cpu << "s, "
									  << "Total time: " << r.time.total << "s"
									  << std::endl;
						}
					},
					[&](error& e) {
						aout(self) << "ERR: " << e << std::endl;
					}
				);
			};

			if(cfg.value == "")
			{
				if(!cfg.json)
				{
					aout(self) << "Enter number to be processed (or 'q' to quit): " << std::endl;
				}

				for(std::string line ; std::getline(std::cin, line) ;)
				{
					if(line.empty()) continue;

					if(line[0] == 'q') break;

					process(line);
				}
			}
			else
			{
				if(!cfg.json)
				{
					aout(self) << "Processing " << cfg.value << std::endl;
				}

				process(cfg.value);
			}
		}
	}

	if(!cfg.json)
	{
		std::cout << "Goodbye." << std::endl;
	}
}

CAF_MAIN(io::middleman);

