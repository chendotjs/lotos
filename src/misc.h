#ifndef _MISC_H__
#define _MISC_H__

typedef enum { TRUE = 1, FALSE = 0 } bool;

#define ERROR (-1)
#define OK (0)

#define CRLF "\r\n"

#define ABORT_ON(cond, msg)                                                                        \
  do {                                                                                             \
    if (cond) {                                                                                    \
      fprintf(stderr, "%s: %d: ", __FILE__, __LINE__);                                             \
      perror(msg);                                                                                 \
      abort();                                                                                     \
    }                                                                                              \
  } while (0)

#define ERR_ON(cond, msg)                                                                          \
  do {                                                                                             \
    if (cond) {                                                                                    \
      fprintf(stderr, "%s: %d: ", __FILE__, __LINE__);                                             \
      perror(msg);                                                                                 \
    }                                                                                              \
  } while (0)

enum { LOG_ERR, LOG_INFO };

extern void lotos_log(int priority, const char *format, ...);

#endif
