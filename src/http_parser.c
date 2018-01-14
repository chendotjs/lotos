#include "buffer.h"
#include "http_parser.h"
#include "misc.h"
#include <string.h>

#define STR2_EQ(p, q) ((p)[0] == (q)[0] && (p)[1] == (q)[1])
#define STR3_EQ(p, q) (STR2_EQ(p, q) && (p)[2] == (q)[2])
#define STR4_EQ(p, q) (STR2_EQ(p, q) && STR2_EQ(p + 2, q + 2))
#define STR5_EQ(p, q) (STR2_EQ(p, q) && STR3_EQ(p + 2, q + 2))
#define STR6_EQ(p, q) (STR3_EQ(p, q) && STR3_EQ(p + 3, q + 3))
#define STR7_EQ(p, q) (STR3_EQ(p, q) && STR4_EQ(p + 3, q + 3))

static int parse_method(char *begin, char *end);

/* use a fsm to parse */
int parse_line(buffer_t *b, parse_settings *st) {
  char ch;
  char *p;
  for (p = st->next_parse_pos; p < buffer_end(b); p++) {
    ch = *p;
    switch (st->state) {
    case RL_BEGIN:
      switch (ch) {
      case 'a' ... 'z':
      case 'A' ... 'Z':
        // TODO: save current pos, which is METHOD beginning
        st->method_begin = p;
        st->state = RL_METHOD;
        break;
      default:
        return INVALID_REQUEST;
      } // end RL_BEGIN

    case RL_METHOD:
      switch (ch) {
      case 'a' ... 'z':
      case 'A' ... 'Z':
        break;
      case ' ': {
        int method = parse_method(st->method_begin, p);
        st->method = method;
        if (st->on_method) {
          st->on_method(method);
        }
        if (method == HTTP_INVALID)
          return INVALID_REQUEST;
        st->state = RL_BEFORE_URI;
        break;
      default:
        return INVALID_REQUEST;
      }
      } // end RL_METHOD
      break;
    // TODO: RL_BEFORE_URI
    case RL_BEFORE_URI:
      switch (ch) {
      case ' ':
        break;
      }
      break;
    } // end switch(state)
  }   // end for
  st->next_parse_pos = buffer_end(b);
  return AGAIN;
}

static int parse_method(char *begin, char *end) {
  int len = end - begin;
  switch (len) {
  case 3:
    if (STR3_EQ(begin, "GET")) {
      return HTTP_GET;
    } else if (STR3_EQ(begin, "PUT")) {
      return HTTP_PUT;
    } else {
      return HTTP_INVALID;
    }
    break;
  case 4:
    if (STR4_EQ(begin, "POST")) {
      return HTTP_POST;
    } else if (STR4_EQ(begin, "HEAD")) {
      return HTTP_HEAD;
    } else {
      return HTTP_INVALID;
    }
    break;
  case 6:
    if (STR6_EQ(begin, "DELETE")) {
      return HTTP_DELETE;
    } else {
      return HTTP_INVALID;
    }
    break;
  default:
    return HTTP_INVALID;
  }
  return HTTP_INVALID;
}
