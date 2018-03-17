#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include "str.h"

struct HTTPClient;

struct HTTPClient* httpclient_new(const char* url);
struct Str httpclient_get(struct HTTPClient* self);
void httpclient_delete(struct HTTPClient* self);

#endif
