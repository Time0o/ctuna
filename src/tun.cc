#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <linux/if.h>
#include <linux/if_tun.h>
#include <linux/route.h>

#include "tun.h"

TUN::~TUN()
{
  if (m_fd != -1)
    ::close(m_fd);

  if (m_sock != -1)
    ::close(m_sock);
}

void TUN::open(std::string const &addr, std::string const &netmask)
{
  // Open /dev/net/tun.
  if ((m_fd = ::open("/dev/net/tun", O_RDWR)) == -1)
    throw std::runtime_error("failed to open /dev/net/tun");

  // Create socket.
  if ((m_sock = ::socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    throw std::runtime_error("failed create socket");

  // Create TUN interface.
  {
    ifreq ifr {};
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

    if (::ioctl(m_fd, TUNSETIFF, &ifr) == -1)
      throw std::runtime_error("failed to create TUN interface");

    std::strncpy(m_name, ifr.ifr_name, IFNAMSIZ);
  }

  // Assign IP address and netmask.
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

  // Set interface properties.
  {
    ifreq ifr {};
    std::strncpy(ifr.ifr_name, m_name, IFNAMSIZ);

    ifr.ifr_flags = IFF_MULTICAST | IFF_NOARP | IFF_UP;

    if (::ioctl(m_sock, SIOCSIFFLAGS, &ifr) == -1)
      throw std::runtime_error("failed to set TUN interface into 'up' state");
  }
}

void TUN::intercept()
{
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

std::vector<std::uint8_t> TUN::read()
{
  static std::uint8_t buf[1024];

  ssize_t bytes_read;
  if ((bytes_read = ::read(m_fd, buf, sizeof(buf)) == -1))
    throw std::runtime_error("failed to read from TUN interface");

  return std::vector<std::uint8_t>(buf, buf + bytes_read);
}
