#define _GNU_SOURCE
#include "server.h"
#include "misc.h"
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>

config_t server_config = {
    .port = 8888,
    .debug = FALSE,
    .timeout = 60,
    .worker = 4,
    .rootdir = NULL,
};

int config_parse(int argc, char *argv[]) {
  int c;
  while ((c = getopt(argc, argv, "p:dt:w:r:")) != -1) {
    switch (c) {
    case 'p':
      server_config.port = atoi(optarg);
      break;
    case 'd':
      server_config.debug = TRUE;
      break;
    case 't':
      server_config.timeout = atoi(optarg);
      break;
    case 'w':
      server_config.worker = atoi(optarg);
      break;
    case 'r':
      server_config.rootdir = optarg;
      break;
    default:
      return ERROR;
    }
  }
  DIR *dirp = NULL;
  if (server_config.rootdir != NULL &&
      (dirp = opendir(server_config.rootdir)) != NULL) {
    closedir(dirp);
    return OK;
  } else
    return ERROR;
}
