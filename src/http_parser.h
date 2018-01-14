#ifndef _PARSER_H__
#define _PARSER_H__

#include "buffer.h"
#include "connection.h"
#include "misc.h"

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

#endif
