#include <iostream>
#include <algorithm>
#include <random>
#include <cstring>

#include "util.h"
#include "socket.h"
#include "sniffer.h"
#include "thread.h"

int main(int argc, char *argv[]) try
{
	using namespace vs;

	std::vector<std::string> args(argv + 1, argv + argc);

	std::cout << "VSP/2 Lab 3" << std::endl;

	if(args.size() < 4)
	{
		std::cout << "usage: ./" << argv[0] << " {iface} {group} {port} {sniffer|listen|client}" << std::endl;
		return 1;
	}

	std::string iface(args[0][0] == '-' ? "" : args[0]);
	std::string group(args[1]);
	uint16_t port = lexical_cast<uint16_t>(args[2]);
	bool quiet = (std::find(args.begin(), args.end(), "-q") != args.end());

	switch(args[3][0])
	{
		case 'l':
		{
			Thread thread;
			Socket sock(port);

			sock.join(group);

			thread = [&](void) {
				sock.listen([&](const Socket::buffer_t& msg) {
					std::string payload((char *) &msg[1], (char *) &msg[10]);

					std::cout << payload << " @" << (now() % 10000) << std::endl;
				});
			};

			for(std::string line ; std::getline(std::cin, line) ;)
			{
				if(line.empty()) continue;

				if(line[0] == 'q') break;
			}
		}
		break;

		case 's':
			Sniffer::run(group, port);
		break;

		case 'c':
		{
			const int slots_per_frame = 25;
			const int frame_length = 1000;
			const int slot_length = frame_length / slots_per_frame;

			byte_t message[34] = {0};
			byte_t schedule[25] = {0};
			bool running = true;

			byte_t buf[2][24] = {0};
			int idx = 0;

			Thread send_thread, receive_thread;
			Socket sender(Socket::getAddressOf(iface));
			Socket receiver(port);

			sender.join(group, iface);
			receiver.join(group);
			
			send_thread = [&](void) {
				while(running)
				{
					for(int i = 0 ; i < 24 ; ++i)
					{
						buf[idx ^ 1][i] = std::cin.get();
					}

					idx ^= 1;
				}
			};

			receive_thread = [&](void) {
				receiver.listen([&](const Socket::buffer_t& msg) {
					const packet_t *packet = reinterpret_cast<const packet_t *>(&msg[0]);

					schedule[(packet->next_slot - SLOT_IDX_OFFSET) % sizeof(schedule)] = 1;
				});
			};

			std::random_device seed;
			std::mt19937 gen(seed());
			std::uniform_int_distribution<uint> rng(0, 24);

			uint next_slot = 0;
			auto find_next_slot = [&](void) {
				for(int j = 0 ; j < 3 ; ++j)
				{
					next_slot = rng(gen);

					if(!schedule[next_slot]) break;
				}

				if(schedule[next_slot])
				{
					std::vector<uint> pot;
					for(uint j = 0 ; j < sizeof(schedule) ; ++j)
					{
						if(!schedule[j])
						{
							pot.push_back(j);
						}
					}
					if(!pot.empty())
					{
						next_slot = pot[next_slot % pot.size()];
					}
				}

				if(schedule[next_slot]) next_slot = sizeof(schedule);

				while(schedule[next_slot])
				{
					if(++next_slot >= sizeof(schedule)) break;
				}
			};

			sleep(500);

			sync(frame_length);

			int slot = sizeof(schedule) - 1;
			for(int i = 0 ; running ; ++i)
			{
				uint64_t next = (now() + frame_length / 2) / frame_length + 1;

				std::memset(schedule, 0, sizeof(schedule));

				for(int j = 0 ; j < slots_per_frame ; ++j)
				{
					uint64_t tnow = (next - 1) * frame_length + j * slot_length + slot_length / 2;

					sleep_until(tnow);

					if(j == slot)
					{
						if(message[0])
						{
							find_next_slot();

							message[25] = next_slot + SLOT_IDX_OFFSET;
							std::memcpy(message + 26, &tnow, 8);

							sender.send(group, port, message, sizeof(message));

							if(!quiet)
							{
								std::cout << std::setw(4) << (now() % 10000) << ": ";
								for(uint k = 0 ; k < sizeof(message) ; ++k)
								{
									std::cout << hex<8>(message[k]) << " ";
								}
								std::cout << std::endl;
							}
						}

						message[0] = 'A';
						std::memcpy(message + 1, &buf[idx][0], 24);
					}
				}

				slot = next_slot;

				sleep_until(next * frame_length);
			}
		}
		break;

		default:
			std::cerr << "Unknown option '" << argv[3] << "'!" << std::endl;
			break;
	}

	return 0;
}
catch(const std::exception& e)
{
	std::cerr << "Caught stray exception: " << e.what() << std::endl;
}

