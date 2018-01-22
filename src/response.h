#ifndef _RESPONSE_H__
#define _RESPONSE_H__

// https://github.com/nodejs/http-parser/blob/b11de0f5c65bcc1b906f85f4df58883b0c133e7b/http_parser.h#L233
/* status code */
#define HTTP_STATUS_MAP(XX)                                                    \
  XX(200, OK, OK)                                                              \
  XX(301, MOVED_PERMANENTLY, Moved Permanently)                                \
  XX(304, NOT_MODIFIED, Not Modified)                                          \
  XX(307, TEMPORARY_REDIRECT, Temporary Redirect)                              \
  XX(308, PERMANENT_REDIRECT, Permanent Redirect)                              \
  XX(400, BAD_REQUEST, Bad Request)                                            \
  XX(401, UNAUTHORIZED, Unauthorized)                                          \
  XX(402, PAYMENT_REQUIRED, Payment Required)                                  \
  XX(403, FORBIDDEN, Forbidden)                                                \
  XX(404, NOT_FOUND, Not Found)                                                \
  XX(405, METHOD_NOT_ALLOWED, Method Not Allowed)                              \
  XX(406, NOT_ACCEPTABLE, Not Acceptable)                                      \
  XX(414, URI_TOO_LONG, URI Too Long)                                          \
  XX(500, INTERNAL_SERVER_ERROR, Internal Server Error)                        \
  XX(501, NOT_IMPLEMENTED, Not Implemented)                                    \
  XX(502, BAD_GATEWAY, Bad Gateway)                                            \
  XX(503, SERVICE_UNAVAILABLE, Service Unavailable)                            \
  XX(504, GATEWAY_TIMEOUT, Gateway Timeout)                                    \
  XX(505, HTTP_VERSION_NOT_SUPPORTED, HTTP Version Not Supported)              \
  XX(511, NETWORK_AUTHENTICATION_REQUIRED, Network Authentication Required)

typedef enum {
#define XX(num, name, string) HTTP_STATUS_##name = num,
  HTTP_STATUS_MAP(XX)
#undef XX
} http_status;

void mime_dict_init();
void mime_dict_free();

#endif
