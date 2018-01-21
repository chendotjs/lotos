#include "http_parser.h"
#include "buffer.h"
#include "misc.h"
#include <assert.h>
#include <string.h>

#define STR2_EQ(p, q) ((p)[0] == (q)[0] && (p)[1] == (q)[1])
#define STR3_EQ(p, q) (STR2_EQ(p, q) && (p)[2] == (q)[2])
#define STR4_EQ(p, q) (STR2_EQ(p, q) && STR2_EQ(p + 2, q + 2))
#define STR5_EQ(p, q) (STR2_EQ(p, q) && STR3_EQ(p + 2, q + 2))
#define STR6_EQ(p, q) (STR3_EQ(p, q) && STR3_EQ(p + 3, q + 3))
#define STR7_EQ(p, q) (STR3_EQ(p, q) && STR4_EQ(p + 3, q + 3))

#define HEADER_SET(header, str_beg, str_end)                                   \
  do {                                                                         \
    assert(str_beg <= str_end);                                                \
    (header)->str = str_beg;                                                   \
    (header)->len = (str_end) - (str_beg);                                     \
  } while (0)

static int parse_method(char *begin, char *end);
static int parse_url(char *begin, char *end, parse_archive *ar);

/* parse request line */
/**
 * @return
 * OK: request line OK
 * AGAIN: parse to the end of buffer, but no complete request line
 * INVALID_REQUEST request not valid
 */
int parse_request_line(buffer_t *b, parse_archive *ar) {
  char ch;
  char *p;
  for (p = ar->next_parse_pos; p < buffer_end(b); p++) {
    ch = *p;
    switch (ar->state) {
    case S_RL_BEGIN:
      switch (ch) {
      case 'a' ... 'z':
      case 'A' ... 'Z':
        /* save current pos, which is METHOD beginning */
        ar->method_begin = p;
        ar->state = S_RL_METHOD;
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
        ar->method = parse_method(ar->method_begin, p);
        if (ar->method == HTTP_INVALID)
          return INVALID_REQUEST;
        ar->state = S_RL_SP_BEFORE_URL;
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
        ar->state = S_RL_URL;
        ar->url_begin = p;
      }
      break;

    case S_RL_URL:
      switch (ch) {
      case ' ':
      case '\t':
        ar->state = S_RL_SP_BEFORE_VERSION;
        int url_status = parse_url(ar->url_begin, p, ar);
        if (url_status)
          return url_status;
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
        ar->state = S_RL_VERSION_H;
        break;
      default:
        return INVALID_REQUEST;
      } // end S_RL_SP_BEFORE_RL_VERSION
      break;
    case S_RL_VERSION_H:
      switch (ch) {
      case 'T':
      case 't':
        ar->state = S_RL_VERSION_HT;
        break;
      default:
        return INVALID_REQUEST;
      } // end S_RL_VERSION_H
      break;
    case S_RL_VERSION_HT:
      switch (ch) {
      case 'T':
      case 't':
        ar->state = S_RL_VERSION_HTT;
        break;
      default:
        return INVALID_REQUEST;
      } // end S_RL_VERSION_HT
      break;
    case S_RL_VERSION_HTT:
      switch (ch) {
      case 'P':
      case 'p':
        ar->state = S_RL_VERSION_HTTP;
        break;
      default:
        return INVALID_REQUEST;
      } // end S_RL_VERSION_HTT
      break;
    case S_RL_VERSION_HTTP:
      switch (ch) {
      case '/':
        ar->state = S_RL_VERSION_HTTP_SLASH;
        break;
      default:
        return INVALID_REQUEST;
      } // end S_RL_VERSION_HTTP
      break;
    case S_RL_VERSION_HTTP_SLASH:
      switch (ch) {
      case '0' ... '9':
        ar->version.http_major = ar->version.http_major * 10 + ch - '0';
        ar->state = S_RL_VERSION_MAJOR;
        break;
      default:
        return INVALID_REQUEST;
      } // end S_RL_VERSION_HTTP_SLASH
      break;
    case S_RL_VERSION_MAJOR:
      switch (ch) {
      case '0' ... '9':
        ar->version.http_major = ar->version.http_major * 10 + ch - '0';
        if (ar->version.http_major > 1)
          return INVALID_REQUEST;
        break;
      case '.':
        ar->state = S_RL_VERSION_DOT;
        break;
      default:
        return INVALID_REQUEST;
      } // end S_RL_VERSION_MAJOR
      break;
    case S_RL_VERSION_DOT:
      switch (ch) {
      case '0' ... '9':
        ar->version.http_minor = ar->version.http_minor * 10 + ch - '0';
        ar->state = S_RL_VERSION_MINOR;
        break;
      default:
        return INVALID_REQUEST;
      } // end S_RL_VERSION_DOT
      break;
    case S_RL_VERSION_MINOR:
      switch (ch) {
      case '0' ... '9':
        ar->version.http_minor = ar->version.http_minor * 10 + ch - '0';
        if (ar->version.http_minor > 1)
          return INVALID_REQUEST;
        break;
      case '\r':
        ar->state = S_RL_CR_AFTER_VERSION;
        break;
      default:
        return INVALID_REQUEST;
      } // end S_RL_VERSION_MINOR
      break;
    case S_RL_CR_AFTER_VERSION:
      switch (ch) {
      case '\n':
        ar->state = S_RL_LF_AFTER_VERSION;
        /* parse request line done*/
        goto done;
      default:
        return INVALID_REQUEST;
      } // end S_RL_CR_AFTER_VERSION
      break;
    } // end switch(state)
  }   // end for
  ar->next_parse_pos = buffer_end(b);
  return AGAIN;
done:;
  ar->next_parse_pos = p + 1;
  ar->state = S_HD_BEGIN;
  return OK;
}

/* parse header line */
/**
 * @return
 *  OK: one header line has been parsed
 *  AGAIN: parse to the end of buffer, but no complete header
 *  INVALID_REQUEST request not valid
 *  CRLF_LINE: `\r\n`, which means all headers have been parsed
 *
 */
int parse_header_line(buffer_t *b, parse_archive *ar) {
  char ch, *p;
  // NOTE: isCRLF_LINE must be an attribute of ar, cannot be a local variable.
  // see the change in fix commit.
  // bool isCRLF_LINE = TRUE;
  for (p = ar->next_parse_pos; p < buffer_end(b); p++) {
    ch = *p;
    switch (ar->state) {
    case S_HD_BEGIN:
      switch (ch) {
      case 'A' ... 'Z':
      case 'a' ... 'z':
      case '0' ... '9':
      case '-':
        ar->state = S_HD_NAME;
        ar->header_line_begin = p;
        ar->isCRLF_LINE = FALSE;
        break;
      case '\r':
        ar->state = S_HD_CR_AFTER_VAL;
        ar->isCRLF_LINE = TRUE;
        break;
      case ' ':
      case '\t':
        break;
      default:
        return INVALID_REQUEST;
      }
      break;

    case S_HD_NAME:
      switch (ch) {
      case 'A' ... 'Z':
      case 'a' ... 'z':
      case '0' ... '9':
      case '-':
        break;
      case ':':
        ar->state = S_HD_COLON;
        ar->header_colon_pos = p;
        break;
      default:
        return INVALID_REQUEST;
      }
      break;

    case S_HD_COLON:
      switch (ch) {
      case ' ':
      case '\t':
        ar->state = S_HD_SP_BEFORE_VAL;
        break;
      case '\r':
      case '\n':
        return INVALID_REQUEST;
      default:
        ar->state = S_HD_VAL;
        ar->header_val_begin = p;
        break;
      }
      break;

    case S_HD_SP_BEFORE_VAL:
      switch (ch) {
      case ' ':
      case '\t':
        break;
      case '\r':
      case '\n':
        return INVALID_REQUEST;
      default:
        ar->state = S_HD_VAL;
        ar->header_val_begin = p;
        break;
      }
      break;

    case S_HD_VAL:
      switch (ch) {
      case '\r':
        ar->header_val_end = p;
        ar->state = S_HD_CR_AFTER_VAL;
        break;
      case '\n':
        ar->state = S_HD_LF_AFTER_VAL;
        break;
      default:
        break;
      }
      break;

    case S_HD_CR_AFTER_VAL:
      switch (ch) {
      case '\n':
        ar->state = S_HD_LF_AFTER_VAL;
        goto done;
      default:
        return INVALID_REQUEST;
      }
      break;
    } // end switch state
  }   // end for
  ar->next_parse_pos = buffer_end(b);
  return AGAIN;
done:;
  ar->next_parse_pos = p + 1;
  ar->state = S_HD_BEGIN;
  ar->num_headers++;

  /* put header name and val into header[2] */
  HEADER_SET(&ar->header[0], ar->header_line_begin, ar->header_colon_pos);
  HEADER_SET(&ar->header[1], ar->header_val_begin, ar->header_val_end);
  return ar->isCRLF_LINE ? CRLF_LINE : OK;
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

/* simple parse url */
static int parse_url(char *begin, char *end, parse_archive *ar) {
  ar->request_url.str = begin;
  ar->request_url.len = end - begin;
  assert(ar->request_url.len >= 0);

  char *p = begin;
  for (; p != end; p++) {
    if (*p == '?') { //  find the firar '?'
      ar->request_path.str = begin;
      ar->request_path.len = p - begin;

      p++;
      ar->query_string.str = p;
      ar->query_string.len = end - p;
      break;
    }
  }
  if (p == end) { // no query_string
    ar->request_path.str = begin;
    ar->request_path.len = p - begin;

    ar->query_string.str = p;
    ar->query_string.len = 0;
  }
  // parse extension
  for (p = end - 1; p != begin; p--) {
    if (*p == '.') {
      ar->mime_extention.str = p + 1;
      ar->mime_extention.len = end - p - 1;
      break;
    } else if (*p == '/')
      break;
  }

  return OK;
}
