#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  int fd;
  if ((fd = open("/dev/net/tun", O_RDWR)) == -1) {
    return -1;
  }

  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

  if (ioctl(fd, TUNSETIFF, &ifr) == -1) {
    close(fd);
    return -1;
  }

  strcpy(tun_name, ifr.ifr_name);

  return fd;
}

int main(int argc, char **argv)
{
  if (cap_check(CAP_NET_ADMIN) == -1) {
    fprintf(stderr, "missing CAP_NET_ADMIN capability");
    return EXIT_FAILURE;
  }

  int tun_fd;
  char tun_name[100];
  if ((tun_fd = tun_open(tun_name)) == -1) {
    fprintf(stderr, "failed to open TUN interface");
    return EXIT_FAILURE;
  }

  printf("opened TUN interface '%s'\n", tun_name);

  close(tun_fd);

  return EXIT_SUCCESS;
}
