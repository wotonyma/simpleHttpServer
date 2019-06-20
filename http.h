//http.h
#ifndef M_HTTP_H_
#define M_HTTP_H_


#include <unordered_map>
using std::unordered_map;
#include <utility>
#include <iostream>
#include <string>

extern const std::string WEB_ROOT;
extern const std::string HOME_PAGE;

extern std::unordered_map<std::string, std::string> stuffix_map;

extern const char *informational_phrases[];
extern const char *success_phrases[];
extern const char *redirection_phrases[];
extern const char *client_error_phrases[];
extern const char *server_error_phrases[];

//const struct response_class response_classes[];


///////////////////////////////////////////////////////////////////////


/* 用枚举类型保存头部方法 */
enum http_request_method
{
	HTTP_REQ_GET     = 1 << 0,
	HTTP_REQ_POST    = 1 << 1,
	HTTP_REQ_HEAD    = 1 << 2,
	HTTP_REQ_PUT     = 1 << 3,
	HTTP_REQ_DELETE  = 1 << 4,
	HTTP_REQ_OPTIONS = 1 << 5,
	HTTP_REQ_TRACE   = 1 << 6,
	HTTP_REQ_CONNECT = 1 << 7,
	HTTP_REQ_PATCH   = 1 << 8
};

enum http_request_error {
  /* Timeout reached, also @see http_connection_set_timeout()*/
  REQ_HTTP_TIMEOUT,
  /* EOF reached */
  REQ_HTTP_EOF,
  /* Error while reading header, or invalid header*/
  REQ_HTTP_INVALID_HEADER,
  /* Error encountered while reading or writing */
  REQ_HTTP_BUFFER_ERROR,
  /* The http_cancel_request() called on this request.*/
  REQ_HTTP_REQUEST_CANCEL,
  /* Body is greater then http_connection_set_max_body_size() */
  REQ_HTTP_DATA_TOO_LONG
};

struct response_class {
  const char *name;
  size_t num_responses;
  const char **responses;
};

const char *http_response_phrase_internal(int code);


#endif