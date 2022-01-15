#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <linux/if.h>

class TUN
{
public:
  ~TUN();

  std::string name() const
  { return m_name; }

  std::string addr() const
  { return m_addr; }

  std::string netmask() const
  { return m_netmask; }

  void open(std::string const &addr, std::string const &netmask);

  void intercept();

  std::vector<std::uint8_t> read();

private:
  int m_fd { -1 };
  int m_sock { -1 };
  char m_name[IFNAMSIZ];
  std::string m_addr;
  std::string m_netmask;
};
