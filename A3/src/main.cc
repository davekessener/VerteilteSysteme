#include <iostream>
#include <algorithm>
#include <random>
#include <cstring>

#include "util.h"
#include "socket.h"
#include "sniffer.h"
#include "station.h"
#include "thread.h"

int main(int argc, char *argv[]) try
{
	using namespace vs;

	std::vector<std::string> args(argv + 1, argv + argc);

	std::cerr << "VSP/2 Lab 3" << std::endl;

	if(args.size() < 3)
	{
		std::cout << "usage: ./" << argv[0] << " {iface} {group} {port} {type}" << std::endl;
		return 1;
	}

	std::string iface(args[0][0] == '-' ? "" : args[0]);
	std::string group(args[1]);
	uint16_t port = lexical_cast<uint16_t>(args[2]);
	char type = ((args.size() == 4 && args[3] == "A") ? 'A' : 'B');

	if(args.size() == 5)
	{
		Clock<>::adjust(lexical_cast<int64_t>(args[4]));
	}

	if(args[0] == "--sniffer")
	{
		Sniffer::run(group, port);
	}
	else
	{
		runStation(group, port, iface, type);
	}

	return 0;
}
catch(const std::exception& e)
{
	std::cerr << "Caught stray exception: " << e.what() << std::endl;
}

