#include <stdio.h>
#include <stdlib.h>

#include <sys/capability.h>

static int cap_have(cap_flag_t cap)
{
  cap_t caps = cap_get_proc();

  cap_flag_value_t ret = 0;
  cap_get_flag(caps, cap, CAP_EFFECTIVE, &ret);

  return ret;
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

  if (!cap_have(CAP_NET_ADMIN)) {
    error("missing CAP_NET_ADMIN capability");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
