#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/capability.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <linux/if.h>
#include <linux/if_tun.h>

static int cap_check(cap_flag_t cap)
{
  cap_t caps = cap_get_proc();

  cap_flag_value_t ret = 0;
  cap_get_flag(caps, cap, CAP_EFFECTIVE, &ret);

  return ret == 1 ? 0 : -1;
}

int tun_open(char *tun_name)
{
  int tun_fd;
  if ((tun_fd = open("/dev/net/tun", O_RDWR)) == -1) {
    return -1;
  }

  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));

  ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

  if (ioctl(tun_fd, TUNSETIFF, &ifr) == -1) {
    close(tun_fd);
    return -1;
  }

  strncpy(tun_name, ifr.ifr_name, IFNAMSIZ);

  return tun_fd;
}

int tun_assign_addr(int tun_fd,
                    char const *tun_name,
                    char const *tun_addr,
                    char const *tun_netmask)
{
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));

  addr.sin_family = AF_INET;

  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));

  strncpy(ifr.ifr_name, tun_name, IFNAMSIZ);

  int s = socket(AF_INET, SOCK_DGRAM, 0);

  // Assign IP address.
  if (inet_pton(addr.sin_family, tun_addr, &addr.sin_addr) != 1)
    return -1;

  ifr.ifr_addr = *(struct sockaddr *)&addr;

  if (ioctl(s, SIOCSIFADDR, &ifr) == -1)
    return -1;

  // Assign netmask.
  if (inet_pton(addr.sin_family, tun_netmask, &addr.sin_addr) != 1) //
    return -1;

  ifr.ifr_netmask = *(struct sockaddr *)&addr;

  if (ioctl(s, SIOCSIFNETMASK, &ifr) == -1)
    return -1;

  return 0;
}

int main(int argc, char **argv)
{
  if (cap_check(CAP_NET_ADMIN) == -1) {
    fprintf(stderr, "missing CAP_NET_ADMIN capability");
    return EXIT_FAILURE;
  }

  int tun_fd;
  char tun_name[IFNAMSIZ];
  if ((tun_fd = tun_open(tun_name)) == -1) {
    fprintf(stderr, "failed to open TUN interface");
    return EXIT_FAILURE;
  }

  printf("opened TUN interface '%s'\n", tun_name);

  char const *tun_addr = "10.0.0.0";
  char const *tun_netmask = "255.255.255.0";
  if (tun_assign_addr(tun_fd, tun_name, tun_addr, tun_netmask) == -1) {
    fprintf(stderr, "failed to assign IP address to TUN interface");
    close(tun_fd);
    return EXIT_FAILURE;
  }

  printf("assigned IP address '%s' to TUN interface\n", tun_addr);

  char buf [1024];
  for (;;) {
    ssize_t num_bytes = read(tun_fd, buf, sizeof(buf));
    printf("read %zd bytes from TUN interface\n", num_bytes);
  }

  close(tun_fd);

  return EXIT_SUCCESS;
}
