#include "response.h"
#include "dict.h"
#include "ssstr.h"

static ssstr_t mime_list[][2] = {
    {SSSTR("word"), SSSTR("application/msword")},
    {SSSTR("pdf"), SSSTR("application/pdf")},
    {SSSTR("zip"), SSSTR("application/zip")},
    {SSSTR("js"), SSSTR("application/javascript")},
    {SSSTR("gif"), SSSTR("image/gif")},
    {SSSTR("jpeg"), SSSTR("image/jpeg")},
    {SSSTR("jpg"), SSSTR("image/jpeg")},
    {SSSTR("png"), SSSTR("image/png")},
    {SSSTR("css"), SSSTR("text/css")},
    {SSSTR("html"), SSSTR("text/html")},
    {SSSTR("htm"), SSSTR("text/html")},
    {SSSTR("txt"), SSSTR("text/plain")},
    {SSSTR("xml"), SSSTR("text/xml")},
    {SSSTR("svg"), SSSTR("image/svg+xml")},
    {SSSTR("mp4"), SSSTR("video/mp4")},
};

static dict_t mime_dict;

void mime_dict_init() {
  size_t nsize = sizeof(mime_list) / sizeof(mime_list[0]);
  int i;
  dict_init(&mime_dict);
  for (i = 0; i < nsize; i++) {
    dict_put(&mime_dict, &mime_list[i][0], &mime_list[i][1]);
  }
}

void mime_dict_free() { dict_free(&mime_dict); }
