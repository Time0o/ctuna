/* libc includes */
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

/* Linux includes */
#include <linux/if.h>
#include <linux/if_tun.h>
#include <linux/route.h>

/* local includes */
#include "tun.h"


CTuna::TUN::TUN(char const *name,
                char const *addr)
:
	_name { name },
	_addr { addr }
{
	/* Open /dev/net/tun. */
	if ((_fd = ::open("/dev/net/tun", O_RDWR)) == -1)
		throw TUN_error { "failed to open /dev/net/tun" };

	/* Create socket. */
	if ((_sock = ::socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		throw TUN_error { "failed create socket" };

	/* Create TUN interface. */
	{
		ifreq ifr {};
		::strncpy(ifr.ifr_name, name, IFNAMSIZ);
		ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

		if (::ioctl(_fd, TUNSETIFF, &ifr) == -1)
			throw TUN_error { "failed to create TUN interface" };

	}

	/* Assign IP address. */
	{
		ifreq ifr {};
		::strncpy(ifr.ifr_name, _name, IFNAMSIZ);

		sockaddr_in addr_ {};
		addr_.sin_family = AF_INET;

		/* Assign IP address. */
		if (::inet_pton(addr_.sin_family, addr, &addr_.sin_addr) != 1)
			throw TUN_error { "inet_pton failed for IP address" };

		ifr.ifr_addr = *reinterpret_cast<sockaddr *>(&addr_);

		if (::ioctl(_sock, SIOCSIFADDR, &ifr) == -1)
			throw TUN_error { "failed to set IP address for TUN interface" };
	}

	/* Set interface properties. */
	{
		ifreq ifr {};
		::strncpy(ifr.ifr_name, _name, IFNAMSIZ);

		ifr.ifr_flags = IFF_MULTICAST | IFF_NOARP | IFF_UP;

		if (::ioctl(_sock, SIOCSIFFLAGS, &ifr) == -1)
			TUN_error { "failed to set TUN interface into 'up' state" };
	}
}


CTuna::TUN::~TUN()
{
	if (_fd != -1)
		::close(_fd);

	if (_sock != -1)
		::close(_sock);
}


void CTuna::TUN::intercept()
{
	/* Set default route to interface. */
	rtentry rt {};

	auto addr_gateway { reinterpret_cast<sockaddr_in *>(&rt.rt_gateway) };
	addr_gateway->sin_family = AF_INET;

	if (::inet_pton(addr_gateway->sin_family, _addr, &addr_gateway->sin_addr) != 1)
		throw TUN_error { "inet_pton failed for IP address" };

	auto addr_dst { reinterpret_cast<sockaddr_in *>(&rt.rt_dst) };
	addr_dst->sin_family = AF_INET;
	addr_dst->sin_addr.s_addr = INADDR_ANY;

	auto addr_genmask { reinterpret_cast<sockaddr_in *>(&rt.rt_genmask) };
	addr_genmask->sin_family = AF_INET;
	addr_genmask->sin_addr.s_addr = INADDR_ANY;

	rt.rt_flags = RTF_UP | RTF_GATEWAY;
	rt.rt_dev = const_cast<char *>(_name);

	if (::ioctl(_sock, SIOCADDRT, &rt) == -1)
		throw TUN_error { "failed to set default route to TUN interface" };
}


size_t CTuna::TUN::read(uint8_t *buf, size_t buf_size)
{
	ssize_t bytes_read;
	if ((bytes_read = ::read(_fd, buf, buf_size)) == -1)
		throw TUN_error { "failed to read from TUN interface" };

	return static_cast<size_t>(bytes_read);
}
