#include "httpclient.h"
#include "log.h"
#include <curl/curl.h>
#include <stdlib.h>

struct HTTPClient
{
	CURL* curl;
};

static size_t writefunc(char* ptr, size_t size, size_t nummembers, void* userdata)
{
	size_t len = size * nummembers;
	struct Str* str = userdata;
	str_appendfmt(str, "%.*s", (int)len, ptr);
	return len;
}

struct HTTPClient* httpclient_new(const char* url)
{
	log_assert(url, "is NULL");

	struct HTTPClient* self = malloc(sizeof(struct HTTPClient));
	if(!self)
	{
		log_error("malloc failed in <%s>", __func__);
	}

	self->curl = curl_easy_init();
	if(!self->curl)
	{
		log_error("Curl failed to initialize");
	}

	curl_easy_setopt(self->curl, CURLOPT_URL, url);
	curl_easy_setopt(self->curl, CURLOPT_WRITEFUNCTION, writefunc);
	return self;
}

struct Str httpclient_get(struct HTTPClient* self)
{
	log_assert(self, "is NULL");

	struct Str str;
	str_ctor(&str, "");
	curl_easy_setopt(self->curl, CURLOPT_WRITEDATA, &str);
	CURLcode result = curl_easy_perform(self->curl);
	if(result)
	{
		log_error("%s", curl_easy_strerror(result));
	}

	return str;
}

void httpclient_delete(struct HTTPClient* self)
{
	log_assert(self, "is NULL");

	curl_easy_cleanup(self->curl);
	free(self);
}

