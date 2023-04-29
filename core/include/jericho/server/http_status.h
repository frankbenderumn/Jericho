#ifndef JERICHO_SERVER_HTTP_STATUS_H_
#define JERICHO_SERVER_HTTP_STATUS_H_

#include <string>
#include <unordered_map>

// 1xx Informational Request
// 2xx Success
// 3xx Redirection
// 4xx Client Error
// 5xx Server Error

/** TODO: Do some research on WebDAV */

enum HttpGroupType {
    HTTP_INFORMATIONAL = 1,
    HTTP_SUCCESS = 2,
    HTTP_REDIRECTION = 3,
    HTTP_CLIENT_ERR = 4,
    HTTP_SERVER_ERR = 5
};

enum HttpStatusType {
    HTTP_CONTINUE = 100,
    HTTP_SWITCH_PROTOCOLS = 101,
    HTTP_PROCESSING = 102,
    
    HTTP_OK = 200,
    HTTP_CREATED = 201,
    HTTP_ACCEPTED = 202,
    HTTP_NON_AUTHORATIVE_INFO = 203,
    HTTP_NO_CONTENT = 204,
    HTTP_RESET_CONTENT = 205,
    HTTP_PARTIAL_CONTENT = 206,
    
    HTTP_MULTIPLE_CHOICES = 300,
    HTTP_MOVED_PERMANENTLY = 301,
    HTTP_FOUND = 302,
    HTTP_SEE_OTHER = 303,
    HTTP_NOT_MODIFIED = 304,
    HTTP_TEMPORARY_REDIRECT = 307,
    HTTP_PERMANENTLY_REDIRECT = 308,
    
    HTTP_BAD_REQUEST = 400,
    HTTP_UNAUTHORIZED = 401,
    HTTP_FORBIDDEN = 403,
    HTTP_NOT_FOUND = 404,
    HTTP_METHOD_NOT_ALLOWED = 405,
    HTTP_NOT_ACCEPTABLE = 406,
    HTTP_PROXY_AUTHENTICATION_REQUIRED = 407,
    HTTP_REQUEST_TIMEOUT = 408,
    HTTP_CONFLICT = 409,
    HTTP_GONE = 410,
    HTTP_LENGTH_REQUIRED = 411,
    HTTP_PRECONDITION_FAILED = 412,
    HTTP_PAYLOAD_TOO_LARGE = 413,
    HTTP_URI_TOO_LONG = 414,
    HTTP_UNSUPPORTED_MEDIA_TYPE = 415,
    HTTP_RANGE_NOT_SATISFIED = 416,
    HTTP_EXPECTATION_FAILED = 417,
    HTTP_UPGRADE_REQUIRED = 426,
    HTTP_PRECONDITION_REQUIRED = 428,
    HTTP_TOO_MANY_REQUESTS = 429,
    HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
    HTTP_UNAVAILABLE_FOR_LEGAL_REASONS = 451,

    HTTP_SERVER_ERROR = 500,
    HTTP_NOT_IMPLEMENTED = 501,
    HTTP_BAD_GATEWAY = 502,
    HTTP_SERVICE_UNAVAILABLE = 503,
    HTTP_GATEWAY_TIMEOUT = 504,
    HTTP_VERSION_NOT_SUPPORTED = 505,
    HTTP_NETWORK_AUTHENTICATION_REQUIRED = 511
};

static std::unordered_map<HttpGroupType, std::string> http_group_to_name_map = {
    {HTTP_INFORMATIONAL, "Informational Request"},
    {HTTP_SUCCESS, "Success"},
    {HTTP_REDIRECTION, "Redirection"},
    {HTTP_CLIENT_ERR, "Client Error"},
    {HTTP_SERVER_ERR, "Server Error"}
};

static std::unordered_map<int, std::string> http_status_to_name_map = {
    { 100, "Continue"},
    { 101, "Switching Protocols"},
    { 102, "Processing (WebDAV)"}, // ??
    { 200, "OK"},
    { 201, "Created"},
    { 202, "Accepted"},
    { 203, "Non-Authoritative Information"},
    { 204, "No Content"},
    { 205, "Reset Content"},
    { 206, "Partial Content"},
    { 300, "Multiple Choices"},
    { 301, "Moved Permanently"},
    { 302, "Found"},
    { 303, "See Other"},
    { 304, "Not Modified"},
    { 307, "Temporary Redirect"},
    { 308, "Permanent Redirect"},
    { 400, "Bad Request"},
    { 401, "Unauthorized"},
    { 403, "Forbidden"},
    { 404, "Not Found"},
    { 405, "Method Not Allowed"},
    { 406, "Not Acceptable"},
    { 407, "Proxy Authentication Required"},
    { 408, "Request Timeout"},
    { 409, "Conflict"},
    { 410, "Gone"},
    { 411, "Length Required"},
    { 412, "Precondition Failed"},
    { 413, "Payload Too Large"},
    { 414, "URI Too Long"},
    { 415, "Unsupported Media Type"},
    { 416, "Range Not Satisfiable"},
    { 417, "Expectation Failed"},
    { 426, "Upgrade Required"},
    { 428, "Precondition Required"},
    { 429, "Too Many Requests"},
    { 431, "Request Header Fields Too Large"},
    { 451, "Unavailable For Legal Reasons"},
    { 500, "Internal Server Error"},
    { 501, "Not Implemented"},
    { 502, "Bad Gateway"},
    { 503, "Service Unavailable"},
    { 504, "Gateway Timeout"},
    { 505, "HTTP Version Not Supported"},
    { 511, "Network Authentication Required"}
};

class HttpStatus {
  public:
    static HttpGroupType group(HttpStatusType type) {
        return static_cast<HttpGroupType>(type / 100);
    }

    static std::string group_to_name(HttpGroupType type) {
        return http_group_to_name_map[type];
    }

    static std::string status_to_name(int status) {
        return http_status_to_name_map[status];
    }

};

#endif