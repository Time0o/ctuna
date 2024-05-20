/* stdcxx includes */
#include <cstdlib>
#include <iostream>

/* local includes */
#include "cap.h"
#include "tun.h"

using namespace CTuna;


void run()
{
	check_cap(CAP_NET_ADMIN);

	TUN tun;

	tun.open("10.0.0.0", "255.255.255.0");

	tun.intercept();

	for (;;) {
		auto packet { tun.read() };
		std::cout << "Read " << packet.size() << " bytes from TUN interface " << tun.name() << std::endl;
	}
}


int main()
{
	try {
		run();

	} catch (std::exception const &e) {
		std::cerr << "error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
