#include <stdint.h>
#include <string>
#include <curl/curl.h>
#include "api/net/net.h"
#include "helpers.h"
#include "errors.h"
#include "lib/json/json.hpp"
#include "lib/clip/clip.h"
#include "api/clipboard/clipboard.h"

using namespace std;
using json = nlohmann::json;

namespace net {

namespace controllers {

struct String {
  char *ptr;
  size_t len;
};

void init_string(struct String *s) {
  s->len = 0;
  s->ptr = (char *)malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct String *s)
{
  size_t new_len = s->len + size*nmemb;
  s->ptr = (char *)realloc(s->ptr, new_len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}

json fetch(const json &input){
    json output;
    CURL *curl;
    CURLcode res;
    if(!helpers::hasRequiredFields(input, {"data"})) {
        output["error"] = errors::makeMissingArgErrorPayload();
        return output;
    }
    string data = input["data"].get<string>();
    struct String s;
    struct curl_slist *headers = NULL;
    curl_global_init(CURL_GLOBAL_ALL);
    int len = data.size();
    char d[len];
    for(int i =0 ;i < len ;i++){
      d[i]=data[i];
    }


    curl = curl_easy_init();
    if(curl) {
        headers = curl_slist_append(headers, "Accept: application/json");
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, d);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{\"key1\":\"value1\",\"key2\":\"value2\"}");

        init_string(&s);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            // fprintf(stderr, "Request failed: %s\n", curl_easy_strerror(res));
            output["returnValue"] = curl_easy_strerror(res);
        } else {
            // printf("Response: %s\n", s.ptr);
            output["returnValue"] = s.ptr;
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    output["success"] = true;
    return output;

}

} // namespace controllers
} // namespace net
