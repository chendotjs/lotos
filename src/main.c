#include "lotos_epoll.h"
#include "misc.h"
#include "server.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static void usage(const char *executable) {
  printf("Usage: %s -r html_root_dir [-p port] "
         "[-t timeout] [-w worker_num] [-d (debug mode)]\n",
         executable);
}

int main(int argc, char *argv[]) {
  if (argc < 2 || config_parse(argc, argv) != OK) {
    usage(argv[0]);
    exit(ERROR);
  }

  if (server_config.debug) {
    goto work;
  }

  int nworker = 0;
  while (TRUE) {
    if (nworker >= server_config.worker) {
      int status;
      waitpid(-1, &status, 0); // wait all children
      if (WIFEXITED(status))
        raise(SIGINT);
      lotos_log(LOG_ERR, "a worker exit, please restart...");
      raise(SIGINT);
    }
    pid_t pid = fork();
    ABORT_ON(pid == -1, "fork");
    if (pid == 0) { // child
      break;        // child ends up in loop and directly goto `work`
    }
    nworker++;
  }

work:;
  int nfds;

  server_setup(server_config.port);

  // TODO: add listen_fd to epoll and test slow_client whether will trigger
  while (TRUE) {
    nfds = lotos_epoll_wait(epoll_fd, lotos_events, MAX_EVENTS, 40);
    ERR_ON(nfds == ERROR, "lotos_epoll_wait");
    if (nfds > 0)
      printf("nfds: %d\n", nfds);
  }

  close(epoll_fd);
  server_shutdown();
  return OK;
}
