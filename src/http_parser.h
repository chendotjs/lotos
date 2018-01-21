#ifndef _PARSER_H__
#define _PARSER_H__

#include "buffer.h"
#include "misc.h"
#include "ssstr.h"
#include <string.h>

/* RFC2616 */
/* https://www.w3.org/Protocols/rfc2616/rfc2616.html */
/* This is a simple implementation */

/**
 *
 * Request       = Request-Line                   ; Section 5.1
                          (( general-header       ; Section 4.5
                         | request-header         ; Section 5.3
                         | entity-header ) CRLF)  ; Section 7.1
                        CRLF
                        [ message-body ]          ; Section 4.3
 */

/**
 *  Request-Line   = Method SP Request-URI SP HTTP-Version CRLF
 */

/**
 *        request-header = Accept                 ; Section 14.1
                      | Accept-Charset           ; Section 14.2
                      | Accept-Encoding          ; Section 14.3
                      | Accept-Language          ; Section 14.4
                      | Authorization            ; Section 14.8
                      | Expect                   ; Section 14.20
                      | From                     ; Section 14.22
                      | Host                     ; Section 14.23
                      | If-Match                 ; Section 14.24
                      | If-Modified-Since        ; Section 14.25
                      | If-None-Match            ; Section 14.26
                      | If-Range                 ; Section 14.27
                      | If-Unmodified-Since      ; Section 14.28
                      | Max-Forwards             ; Section 14.31
                      | Proxy-Authorization      ; Section 14.34
                      | Range                    ; Section 14.35
                      | Referer                  ; Section 14.36
                      | TE                       ; Section 14.39
                      | User-Agent               ; Section 14.43
 */

#define INVALID_REQUEST (-1)
#define CRLF_LINE (2)

#define MAX_ELEMENT_SIZE (2048)

/* basic http method */
typedef enum {
  HTTP_DELETE,
  HTTP_GET,
  HTTP_HEAD,
  HTTP_POST,
  HTTP_PUT,
  HTTP_INVALID,
} http_method;

/* HTTP protocol version */
typedef struct {
  unsigned short http_major;
  unsigned short http_minor;
} http_version;

/* parser state used in fsm */
typedef enum {
  /* request line states */
  S_RL_BEGIN = 0,
  S_RL_METHOD,
  S_RL_SP_BEFORE_URL,
  S_RL_URL,
  S_RL_SP_BEFORE_VERSION,
  S_RL_VERSION_H,
  S_RL_VERSION_HT,
  S_RL_VERSION_HTT,
  S_RL_VERSION_HTTP,
  S_RL_VERSION_HTTP_SLASH,
  S_RL_VERSION_MAJOR,
  S_RL_VERSION_DOT,
  S_RL_VERSION_MINOR,
  S_RL_CR_AFTER_VERSION,
  S_RL_LF_AFTER_VERSION,

  /* header states */
  S_HD_BEGIN,
  S_HD_NAME,
  S_HD_COLON,
  S_HD_SP_BEFORE_VAL,
  S_HD_VAL,
  S_HD_CR_AFTER_VAL,
  S_HD_LF_AFTER_VAL,
} parser_state;

typedef struct {
  /* parsed request line result */
  http_method method;
  http_version version;
  ssstr_t request_url;
  ssstr_t request_path;
  ssstr_t query_string;
  char mime_extention[MAX_ELEMENT_SIZE / 16];

  /* parsed header lines result */
  bool keep_alive;
  int content_length;
  int num_headers;
  ssstr_t header[2]; /* store header every time` parse_header_line` */

  /* preserve buffer_t state, so when recv new data, we can keep parsing */
  char *next_parse_pos; /* parser position in buffer_t */
  int state;            /* parser state */

  /* private members, do not modify !!! */
  char *method_begin;
  char *url_begin;
  char *header_line_begin;
  char *header_colon_pos;
  char *header_val_begin;
  char *header_val_end;
} parse_archive;

static inline void parse_archive_init(parse_archive *ar, buffer_t *b) {
  memset(ar, 0, sizeof(parse_archive));
  ar->next_parse_pos = b->buf;
}

/* status code */
#define HTTP_STATUS_MAP(GEN)                                                   \
  GEN(200, OK, OK)                                                             \
  GEN(301, MOVED_PERMANENTLY, Moved Permanently)                               \
  GEN(304, NOT_MODIFIED, Not Modified)                                         \
  GEN(307, TEMPORARY_REDIRECT, Temporary Redirect)                             \
  GEN(308, PERMANENT_REDIRECT, Permanent Redirect)                             \
  GEN(400, BAD_REQUEST, Bad Request)                                           \
  GEN(401, UNAUTHORIZED, Unauthorized)                                         \
  GEN(402, PAYMENT_REQUIRED, Payment Required)                                 \
  GEN(403, FORBIDDEN, Forbidden)                                               \
  GEN(404, NOT_FOUND, Not Found)                                               \
  GEN(405, METHOD_NOT_ALLOWED, Method Not Allowed)                             \
  GEN(406, NOT_ACCEPTABLE, Not Acceptable)                                     \
  GEN(414, URI_TOO_LONG, URI Too Long)                                         \
  GEN(500, INTERNAL_SERVER_ERROR, Internal Server Error)                       \
  GEN(501, NOT_IMPLEMENTED, Not Implemented)                                   \
  GEN(502, BAD_GATEWAY, Bad Gateway)                                           \
  GEN(503, SERVICE_UNAVAILABLE, Service Unavailable)                           \
  GEN(504, GATEWAY_TIMEOUT, Gateway Timeout)                                   \
  GEN(505, HTTP_VERSION_NOT_SUPPORTED, HTTP Version Not Supported)             \
  GEN(511, NETWORK_AUTHENTICATION_REQUIRED, Network Authentication Required)

typedef enum {
#define GEN(num, name, string) HTTP_STATUS_##name = num,
  HTTP_STATUS_MAP(GEN)
#undef GEN
} http_status;

extern int parse_request_line(buffer_t *b, parse_archive *ar);
extern int parse_header_line(buffer_t *b, parse_archive *ar);

#endif
