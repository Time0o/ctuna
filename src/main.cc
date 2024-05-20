/* libc */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* local includes */
#include "cap.h"
#include "exception.h"
#include "tun.h"

using namespace CTuna;


void run(char const *tun_name, char const *tun_addr)
{
	check_cap(CAP_NET_ADMIN);

	TUN tun { tun_name, tun_addr };

	tun.intercept();

	uint8_t buf[1024];

	for (;;) {
		auto bytes_read { tun.read(buf, sizeof(buf)) };
		::printf("Read %zu bytes from TUN interface %s\n", bytes_read, tun.name());
	}
}


int main(int argc, char **argv)
{
	if (argc != 3) {
		::fprintf(stderr, "Usage: %s TUN_NAME TUN_ADDR\n", argv[0]);
		return EXIT_FAILURE;
	}

	try {
		run(argv[1], argv[2]);

	} catch (Exception const &e) {
		::fprintf(stderr, "Error: %s\n", e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
