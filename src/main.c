#include "misc.h"
#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void usage(const char *executable) {
  printf("Usage: %s -r html_root_dir [-p port] "
         "[-d] [-t timeout] [-w worker_num]\n",
         executable);
}

int main(int argc, char *argv[]) {
  if (argc < 2 || config_parse(argc, argv) != OK) {
    usage(argv[0]);
    exit(ERROR);
  }

  return 0;
}
