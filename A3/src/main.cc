#include <iostream>
#include <random>
#include <cstring>

#include "util.h"
#include "socket.h"
#include "sniffer.h"

int main(int argc, char *argv[]) try
{
	using namespace vs;

	std::cout << "VSP/2 Lab 3" << std::endl;

	if(argc != 4)
	{
		std::cout << "usage: ./" << argv[0] << " {group} {port} {sniffer|listen|client}" << std::endl;
		return 1;
	}

	std::string group(argv[1]);
	uint16_t port = lexical_cast<uint16_t>(argv[2]);

	switch(argv[3][0])
	{
		case 'l':
		{
			Socket *sock = new Socket(port);

			sock->join(group);

			int count = 5;
			sock->listen([&](const Socket::buffer_t& msg) {
				std::cout << std::string((char *) &msg[1], (char *) &msg[10]) << " @" << (now() % 10000) << std::endl;

				if(!--count)
				{
					delete sock; sock = nullptr;
				}
			});
		}
		break;

		case 's':
		{
			const uint delta = SLOT_DURATION / 3;

			uint64_t last_frame = 0;
			uint last_slot = SLOTS_PER_FRAME;

			Sniffer snif([&](const packet_t& msg) {
				uint64_t t = now();
				uint64_t frame = (t + FRAME_DURATION - 1) / FRAME_DURATION;
				uint slot = (t % FRAME_DURATION) / SLOT_DURATION;
				std::string teamname((const char *) &msg.name[0], (const char *) &msg.name[sizeof(msg.name)]);
				int next_slot = msg.next_slot - SLOT_IDX_OFFSET;
				uint offset = ((t % FRAME_DURATION) % SLOT_DURATION);
				bool collision = (last_frame == frame && last_slot == slot);
				bool middle = ((delta < offset) && (offset < SLOT_DURATION - delta));

				if(last_frame != frame)
				{
					std::cout << stringify("=== Frame ", std::setw(12), frame, " ===\n")
							  << std::flush;
				}

				last_slot = slot;
				last_frame = frame;

				std::cout << stringify(
					(collision ? 'X' : '-'),
					(!middle ? 'O' : '-'),
					std::setw(15), t,
					" (Slot ",
					std::setw(2), slot,
					"): received '", teamname,
					"' [", msg.type, "] next slot: ",
					std::setw(2), next_slot,
					" TX: ", msg.timestamp, "\n")
				<< std::flush;
			});

			snif.start(group, port);

			for(std::string line ; std::getline(std::cin, line) ;)
			{
				if(line.empty()) continue;

				if(line[0] == 'q') break;
			}

			snif.stop();
		}
		break;

		case 'c':
		{
			Socket sock;

			const int frame_count = 100;
			const int slots_per_frame = 25;
			const int frame_length = 1000;
			const int slot_length = frame_length / slots_per_frame;

			byte_t message[34] = {0};
			bool running = true;

			byte_t buf[2][24] = {0};
			int idx = 0;

			std::thread thread([&running, &idx, &buf](void) {
				while(running)
				{
					for(int i = 0 ; i < 24 ; ++i)
					{
						buf[idx ^ 1][i] = std::cin.get();
					}

					idx ^= 1;
				}
			});

			std::default_random_engine gen;
			std::uniform_int_distribution<uint> rng(0, 24);

			sleep(500);

			sync(frame_length);

			int slot = 12;
			for(int i = 0 ; i <= frame_count ; ++i)
			{
				uint64_t next = (now() + frame_length / 2) / frame_length + 1;
				uint next_slot = rng(gen);

				message[25] = next_slot + SLOT_IDX_OFFSET;

				for(int j = 0 ; j < slots_per_frame ; ++j)
				{
					uint64_t tnow = (next - 1) * frame_length + j * slot_length + slot_length / 2;

					sleep_until(tnow);

					if(j == slot)
					{
						if(message[0])
						{
							std::memcpy(message + 26, &tnow, 8);
							sock.send(group, port, message, sizeof(message));

							std::cout << std::setw(4) << (now() % 10000) << ": ";
							for(uint k = 0 ; k < sizeof(message) ; ++k)
							{
								std::cout << hex<8>(message[k]) << " ";
							}
							std::cout << std::endl;
						}

						message[0] = 'A';
						std::memcpy(message + 1, &buf[idx][0], 24);
					}
				}

				slot = next_slot;

				sleep_until(next * frame_length);
			}

			running = false;
			thread.join();
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

