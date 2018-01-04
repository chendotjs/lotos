#ifndef _SERVER_H__
#define _SERVER_H__
#include "misc.h"
#include <stdint.h>

typedef struct {
  uint16_t port;   /* listen port */
  bool debug;      /* debug mode */
  int timeout;     /* connection expired time */
  uint32_t worker; /* worker num */
  char *rootdir;   /* html root directory */
} config_t;

extern config_t server_config;

extern int config_parse(int argc, char *argv[]);

#endif
