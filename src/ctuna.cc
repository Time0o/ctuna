#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/capability.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <linux/if.h>
#include <linux/if_tun.h>

namespace {

void check_cap(cap_value_t cap)
{
  cap_t caps { ::cap_get_proc() };

  cap_flag_value_t flag { static_cast<cap_flag_value_t>(0) };
  ::cap_get_flag(caps, cap, CAP_EFFECTIVE, &flag);

  if (!flag)
    throw std::runtime_error("missing capability");
}

} // end namespace

class TUN
{
public:
  void open(char const *addr, char const *netmask)
  {
    if ((m_fd = ::open("/dev/net/tun", O_RDWR)) == -1)
      throw std::runtime_error("failed to open /dev/net/tun");

    if ((m_sock = ::socket(AF_INET, SOCK_DGRAM, 0)) == -1)
      throw std::runtime_error("failed create socket");

    {
      ifreq ifr {};
      ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

      if (::ioctl(m_fd, TUNSETIFF, &ifr) == -1)
        throw std::runtime_error("failed to create TUN interface");

      std::strncpy(m_name, ifr.ifr_name, IFNAMSIZ);
    }

    {
      ifreq ifr {};
      std::strncpy(ifr.ifr_name, m_name, IFNAMSIZ);

      sockaddr_in addr_ {};
      addr_.sin_family = AF_INET;

      // Assign IP address.
      if (::inet_pton(addr_.sin_family, addr, &addr_.sin_addr) != 1)
        throw std::runtime_error("inet_pton failed for IP address");

      ifr.ifr_addr = *reinterpret_cast<sockaddr *>(&addr_);

      if (::ioctl(m_sock, SIOCSIFADDR, &ifr) == -1)
        throw std::runtime_error("failed to set IP address for TUN interface");

      // Assign netmask.
      if (::inet_pton(addr_.sin_family, netmask, &addr_.sin_addr) != 1)
        throw std::runtime_error("inet_pton failed for netmask");

      ifr.ifr_netmask = *reinterpret_cast<sockaddr *>(&addr_);

      if (::ioctl(m_sock, SIOCSIFNETMASK, &ifr) == -1)
        throw std::runtime_error("failed to set netmask for TUN interface");
    }
  }

private:
  int m_fd { -1 };
  int m_sock { -1 };
  char m_name[IFNAMSIZ];
};

void run()
{
  check_cap(CAP_NET_ADMIN);

  TUN tun;
  tun.open("10.0.0.0", "255.255.255.0");
}

int main()
{
  try {
    run();
  }
  catch (std::exception const &e) {
    std::cerr << "error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
