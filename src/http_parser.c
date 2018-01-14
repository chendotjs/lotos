#include "buffer.h"
#include "http_parser.h"
#include "misc.h"


//
int parse_line(buffer_t *b, int *pbuf_cur_pos, int *pcur_state) {
  char ch;
  int i;
  for (i = *pbuf_cur_pos; i < b->len; i++) {
    ch = b->buf[i];
    switch (*pcur_state) {
    case RL_BEGIN:
      switch (ch) {
      case 'a' ... 'z':
      case 'A' ... 'Z':
        // TODO: save current pos, which is METHOD beginning
        *pcur_state = RL_METHOD;
        break;
      default:
        return INVALID_REQUEST;
      } // end RL_BEGIN

    case RL_METHOD:
      switch (ch) {
      case 'a' ... 'z':
      case 'A' ... 'Z':
        break;
      } // end RL_METHOD
    }   // end cur_state
  }     // end for
  *pbuf_cur_pos = b->len;
  return AGAIN;
}
