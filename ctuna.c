#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/capability.h>
#include <sys/ioctl.h>

#include <linux/if.h>
#include <linux/if_tun.h>

static int cap_check(cap_flag_t cap)
{
  cap_t caps = cap_get_proc();

  cap_flag_value_t ret = 0;
  cap_get_flag(caps, cap, CAP_EFFECTIVE, &ret);

  return ret == 1 ? 0 : -1;
}

int tun_open(char *name)
{
  int fd;
  if ((fd = open("/dev/net/tun", O_RDWR)) == -1) {
    return -1;
  }

  struct ifreq ifr;
  strncpy(ifr.ifr_name, name, IFNAMSIZ);
  ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

  if (ioctl(fd, TUNSETIFF, &ifr) == -1) {
    close(fd);
    return -1;
  }

  return fd;
}

static char *progname;

static void info(char const *msg)
{
  printf("%s\n", msg);
}

static void error(char const *msg)
{
  fprintf(stderr, "%s: error: %s\n", progname, msg);
}

int main(int argc, char **argv)
{
  progname = argv[0];

  if (cap_check(CAP_NET_ADMIN) == -1) {
    error("missing CAP_NET_ADMIN capability");
    return EXIT_FAILURE;
  }

  int tun_fd;
  if ((tun_fd = tun_open("ctuna")) == -1) {
    error("failed to open TUN interface");
    return EXIT_FAILURE;
  }

  close(tun_fd);

  return EXIT_SUCCESS;
}
