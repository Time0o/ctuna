#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/capability.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <linux/if.h>
#include <linux/if_tun.h>
#include <linux/route.h>

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
  ~TUN()
  {
    if (m_fd != -1)
      ::close(m_fd);

    if (m_sock != -1)
      ::close(m_sock);
  }

  std::string name() const
  { return m_name; }

  std::string addr() const
  { return m_addr; }

  std::string netmask() const
  { return m_netmask; }

  void open(std::string const &addr, std::string const &netmask)
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
      if (::inet_pton(addr_.sin_family, addr.c_str(), &addr_.sin_addr) != 1)
        throw std::runtime_error("inet_pton failed for IP address");

      ifr.ifr_addr = *reinterpret_cast<sockaddr *>(&addr_);

      if (::ioctl(m_sock, SIOCSIFADDR, &ifr) == -1)
        throw std::runtime_error("failed to set IP address for TUN interface");

      m_addr = addr;

      // Assign netmask.
      if (::inet_pton(addr_.sin_family, netmask.c_str(), &addr_.sin_addr) != 1)
        throw std::runtime_error("inet_pton failed for netmask");

      ifr.ifr_netmask = *reinterpret_cast<sockaddr *>(&addr_);

      if (::ioctl(m_sock, SIOCSIFNETMASK, &ifr) == -1)
        throw std::runtime_error("failed to set netmask for TUN interface");

      m_netmask = netmask;
    }
  }

  void intercept()
  {
    // Set interface into 'up' state.
    ifreq ifr {};
    std::strncpy(ifr.ifr_name, m_name, IFNAMSIZ);

    ifr.ifr_flags = IFF_UP;

    if (::ioctl(m_sock, SIOCSIFFLAGS, &ifr) == -1)
      throw std::runtime_error("failed to set TUN interface into 'up' state");

    // Set default route to interface.
    rtentry rt {};

    auto addr_gateway { reinterpret_cast<sockaddr_in *>(&rt.rt_gateway) };
    addr_gateway->sin_family = AF_INET;

    if (::inet_pton(addr_gateway->sin_family, m_addr.c_str(), &addr_gateway->sin_addr) != 1)
      throw std::runtime_error("inet_pton failed for IP address");

    auto addr_dst { reinterpret_cast<sockaddr_in *>(&rt.rt_dst) };
    addr_dst->sin_family = AF_INET;
    addr_dst->sin_addr.s_addr = INADDR_ANY;

    auto addr_genmask { reinterpret_cast<sockaddr_in *>(&rt.rt_genmask) };
    addr_genmask->sin_family = AF_INET;
    addr_genmask->sin_addr.s_addr = INADDR_ANY;

    rt.rt_flags = RTF_UP | RTF_GATEWAY;
    rt.rt_dev = m_name;

    if (::ioctl(m_sock, SIOCADDRT, &rt) == -1)
      throw std::runtime_error("failed to set default route to TUN interface");
  }

  std::vector<std::uint8_t> read()
  {
    static std::uint8_t buf[1024];

    ssize_t bytes_read;
    if ((bytes_read = ::read(m_fd, buf, sizeof(buf)) == -1))
      throw std::runtime_error("failed to read from TUN interface");

    return std::vector<std::uint8_t>(buf, buf + bytes_read);
  }

private:
  int m_fd { -1 };
  int m_sock { -1 };
  char m_name[IFNAMSIZ];
  std::string m_addr;
  std::string m_netmask;
};

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
  }
  catch (std::exception const &e) {
    std::cerr << "error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
