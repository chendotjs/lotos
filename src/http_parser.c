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
static int parse_url(char *begin, char *end, parse_settings *st);

/* parse request line */
int parse_request_line(buffer_t *b, parse_settings *st) {
  char ch;
  char *p;
  for (p = st->next_parse_pos; p < buffer_end(b); p++) {
    ch = *p;
    switch (st->state) {
    case S_RL_BEGIN:
      switch (ch) {
      case 'a' ... 'z':
      case 'A' ... 'Z':
        /* save current pos, which is METHOD beginning */
        st->method_begin = p;
        st->state = S_RL_METHOD;
        break;
      default:
        return INVALID_REQUEST;
      } // end S_RL_BEGIN

    case S_RL_METHOD:
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
        st->state = S_RL_SP_BEFORE_URL;
        break;
      default:
        return INVALID_REQUEST;
      }
      } // end S_RL_METHOD
      break;
    case S_RL_SP_BEFORE_URL:
      switch (ch) {
      case ' ':
      case '\t': /* ease parser, '\t' is also considered valid */
        break;
      case '\r':
      case '\n':
        return INVALID_REQUEST;
      default:
        st->state = S_RL_URL;
        st->url_begin = p;
      }
      break;

    case S_RL_URL:
      switch (ch) {
      case ' ':
      case '\t':
        st->state = S_RL_SP_BEFORE_VERSION;
        if (parse_url(st->url_begin, p, st))
          return INVALID_REQUEST;
        break;
      case '\r':
      case '\n':
        return INVALID_REQUEST;
      default:
        break;
      } // end S_RL_URL
      break;
    case S_RL_SP_BEFORE_VERSION:
      switch (ch) {
      case ' ':
      case '\t':
        break;
      case 'H':
      case 'h':
        st->state = S_RL_VERSION_H;
        break;
      default:
        return INVALID_REQUEST;
      } // end S_RL_SP_BEFORE_RL_VERSION
      break;
    case S_RL_VERSION_H:
      switch (ch) {
      case 'T':
      case 't':
        st->state = S_RL_VERSION_HT;
        break;
      default:
        return INVALID_REQUEST;
      } // end S_RL_VERSION_H
      break;
    case S_RL_VERSION_HT:
      switch (ch) {
      case 'T':
      case 't':
        st->state = S_RL_VERSION_HTT;
        break;
      default:
        return INVALID_REQUEST;
      } // end S_RL_VERSION_HT
      break;
    case S_RL_VERSION_HTT:
      switch (ch) {
      case 'P':
      case 'p':
        st->state = S_RL_VERSION_HTTP;
        break;
      default:
        return INVALID_REQUEST;
      } // end S_RL_VERSION_HTT
      break;
    case S_RL_VERSION_HTTP:
      switch (ch) {
      case '/':
        st->state = S_RL_VERSION_HTTP_SLASH;
        break;
      default:
        return INVALID_REQUEST;
      } // end S_RL_VERSION_HTTP
      break;
    case S_RL_VERSION_HTTP_SLASH:
      switch (ch) {
      case '0' ... '9':
        st->version.http_major = st->version.http_major * 10 + ch - '0';
        st->state = S_RL_VERSION_MAJOR;
        break;
      default:
        return INVALID_REQUEST;
      } // end S_RL_VERSION_HTTP_SLASH
      break;
    case S_RL_VERSION_MAJOR:
      switch (ch) {
      case '0' ... '9':
        st->version.http_major = st->version.http_major * 10 + ch - '0';
        if (st->version.http_major > 1)
          return INVALID_REQUEST;
        break;
      case '.':
        st->state = S_RL_VERSION_DOT;
        break;
      default:
        return INVALID_REQUEST;
      } // end S_RL_VERSION_MAJOR
      break;
    case S_RL_VERSION_DOT:
      switch (ch) {
      case '0' ... '9':
        st->version.http_minor = st->version.http_minor * 10 + ch - '0';
        st->state = S_RL_VERSION_MINOR;
        break;
      default:
        return INVALID_REQUEST;
      } // end S_RL_VERSION_DOT
      break;
    case S_RL_VERSION_MINOR:
      switch (ch) {
      case '0' ... '9':
        st->version.http_minor = st->version.http_minor * 10 + ch - '0';
        if (st->version.http_minor > 1)
          return INVALID_REQUEST;
        break;
      case '\r':
        st->state = S_RL_CR_AFTER_VERSION;
        break;
      default:
        return INVALID_REQUEST;
      } // end S_RL_VERSION_MINOR
      break;
    case S_RL_CR_AFTER_VERSION:
      switch (ch) {
      case '\n':
        st->state = S_RL_LF_AFTER_VERSION;
        /* parse request line done*/
        goto done;
        break;
      default:
        return INVALID_REQUEST;
      } // end S_RL_CR_AFTER_VERSION
      break;
    } // end switch(state)
  }   // end for
  st->next_parse_pos = buffer_end(b);
  return AGAIN;
done:;
  st->next_parse_pos = p + 1;
  st->state = S_HD_BEGIN;
  return OK;
}

/* parse header line */
int parse_header_line(buffer_t *b, parse_settings *st) {

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

// TODO: parse URL
static int parse_url(char *begin, char *end, parse_settings *st) {
  size_t len = end - begin;
  if (len < sizeof(st->request_url)) {
    memcpy(st->request_url, begin, len);
    st->request_url[len] = '\0';
  } else
    return INVALID_REQUEST; /* url too long */

  return OK;
}
