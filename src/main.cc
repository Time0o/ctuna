/* libc */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* local includes */
#include "cap.h"
#include "exception.h"
#include "tun.h"

using namespace CTuna;


void run()
{
	check_cap(CAP_NET_ADMIN);

	TUN tun;

	tun.open("10.0.0.0", "255.255.255.0");

	tun.intercept();

	uint8_t buf[1024];

	for (;;) {
		auto bytes_read { tun.read(buf, sizeof(buf)) };
		::printf("Read %zu bytes from TUN interface %s\n", bytes_read, tun.name());
	}
}


int main()
{
	try {
		run();

	} catch (Exception const &e) {
		::fprintf(stderr, "error: %s\n", e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
