#ifndef JERICHO_SERVER_RESPONSE_DEFS_H_
#define JERICHO_SERVER_RESPONSE_DEFS_H_

typedef enum {
    H_ACCEPT,
    H_ACCEPT_CH,
    H_ACCEPT_CHARSET,
    H_ACCEPT_ENC,
    H_ACCEPT_LANG,
    H_ACCEPT_PATCH,
    H_ACCEPT_POST,
    H_ACCEPT_RANGES,
    H_ACCESS_CTRL_ALLOW_CREDS,
    H_ACCESS_CTRL_ALLOW_HEADERS,
    H_ACCESS_CTRL_ALLOW_METHODS,
    H_ACCESS_CTRL_ALLOW_ORIGIN,
    H_ACCESS_CTRL_EXPOSE_HEADERS,
    H_ACCESS_CTRL_MAX_AGE,
    H_ACCESS_CTRL_REQ_HEADERS,
    H_ACCESS_CTRL_REQ_METHOD,
    H_AGE,
    H_ALLOW,
    H_ALT_SVC,
    H_AUTHORIZATION,
    H_CACHE_CTRL,
    H_CLR_SITE_DATA,
    H_CONN,
    H_CONTENT_ENC,
    H_CONTENT_LANG,
    H_CONTENT_LEN,
    H_CONTENT_LOC,
    H_CONTENT_RANGE,
    H_CONTENT_SECURITY_POLICY,
    H_CONTENT_SECURITY_POLICY_REPORT_ONLY,
    H_CONTENT_TYPE,
    H_COOKIE,
    H_CROSS_ORIGIN_EMBEDDER_POLICY,
    H_CROSS_ORIGIN_OPENER_POLICY,
    H_CROSS_ORIGIN_RESOURCE_POLICY,
    H_DATE,
    H_DIGEST,
    H_DNT,
    H_ETAG,
    H_EXPECT,
    H_EXPECT_CT,
    H_EXPIRES,
    H_FORWARDED,
    H_FROM,
    H_HOST,
    H_IF_MATCH,
    H_IF_MODIFIED_SINCE,
    H_IF_NONE_MATCH,
    H_IF_RANGE,
    H_IF_UNMODIFIED_SINCE,
    H_KEEP_ALIVE,
    H_LAST_MODIFIED,
    H_LINK,
    H_LOCATION,
    H_NEL,
    H_ORIGIN,
    H_PROXY_AUTHENTICATE,
    H_PROXY_AUTHORIZATION,
    H_RANGE,
    H_REFERER,
    H_REFERRER_POLICY,
    H_RETRY_AFTER,
    H_SAVE_DATA,
    H_SEC_FETCH_DEST,
    H_SEC_FETCH_MODE,
    H_SEC_FETCH_SITE,
    H_SEC_FETCH_USER,
    H_SEC_WS_ACCEPT,
    H_SERVER,
    H_SERVER_TIMING,
    H_SERVIVE_WORKER_NAVIGATION_PRELOAD,
    H_SET_COOKIE,
    H_SOURCEMAP,
    H_STRICT_TRANSPORT_SECURITY,
    H_TE,
    H_TIMING_ALLOW_ORIGIN,
    H_TK,
    H_TRAILER,
    H_TRANSFER_ENC,
    H_UPGRADE,
    H_UPGRADE_INSECURE_REQUESTS,
    H_USER_AGENT,
    H_VARY,
    H_VIA,
    H_WANT_DIGEST,
    H_WWW_AUTHENTICATE,
    H_X_CONTENT_TYPE_OPTIONS,
    H_X_DNS_PREFETCH_CONTROL,
    H_X_FRAME_OPTIONS,
    H_X_XSS_PROTECTION,
    H_END
} HttpHeaderType;

typedef enum {
    R200,
    R201,
    R202,
    R203,
    R204,
    R205,
    R206,
    R300,
    R301,
    R302,
    R303,
    R304,
    R305,
    R306,
    R307,
    R400,
    R401,
    R402,
    R403,
    R404,
    R405, 
    R406,
    R407,
    R408,
    R409,
    R410,
    R411,
    R412,
    R413,
    R414,
    R415,
    R416,
    R417,
    R500,
    R501,
    R502,
    R503,
    R504,
    R505
} HttpResponseType;

static const char* http_response[] = {
    // Accepted
    "200 OK",
    "201 Created",
    "202 Accepted",
    "203 Non-Authorization Information",
    "204 No Content",
    "205 Reset Content",
    "206 Partial Content",
    // Redirection
    "300 Multiple Choices",
    "301 Moved Permanently",
    "302 Found",
    "303 See Other",
    "304 Not Modified",
    "305 Use Proxy",
    "306 (Unused)",
    "307 Temporary Direct",
    // Client Error
    "400 Bad Request",
    "401 Unauthorized",
    "402 Payment Required",
    "403 Forbidden",
    "404 Not Found",
    "405 Method Not Allowed",
    "406 Not Acceptable",
    "407 Proxy Authentication Required",
    "408 Request Timeout",
    "409 Conflict",
    "410 Gone",
    "411 Length Required",
    "412 Precondition Failed",
    "413 Request Entity Too Large",
    "414 Request-Uri Too Long",
    "415 Unsupported Media Type",
    "416 Requested Range Not Satisfiable",
    "417 Expectation Failed",
    // Server Error
    "500 Internal Server Error",
    "501 Not Implemented",
    "502 Bad Gateway",
    "503 Service Unavailable",
    "504 Gateway Timeout",
    "505 HTTP Version Not Unsupported"
};

static const char* http_header[92] = {
"Accept",
"Accept-CH",
// "ExperimentalNon-StandardDeprecated"
// "Accept-CH-Lifetime"
"Accept-Charset",
"Accept-Encoding",
"Accept-Language",
"Accept-Patch",
"Accept-Post",
"Accept-Ranges",
"Access-Control-Allow-Credentials",
"Access-Control-Allow-Headers",
"Access-Control-Allow-Methods",
"Access-Control-Allow-Origin",
"Access-Control-Expose-Headers",
"Access-Control-Max-Age",
"Access-Control-Request-Headers",
"Access-Control-Request-Method",
"Age",
"Allow",
"Alt-Svc",
"Authorization",
"Cache-Control",
"Clear-Site-Data",
"Connection",
"Content-Disposition",
// "ExperimentalNon-StandardDeprecated"
// "Content-DPR"
"Content-Encoding",
"Content-Language",
"Content-Length",
"Content-Location",
"Content-Range",
"Content-Security-Policy",
"Content-Security-Policy-Report-Only",
"Content-Type",
"Cookie",
"Cross-Origin-Embedder-Policy",
"Cross-Origin-Opener-Policy",
"Cross-Origin-Resource-Policy",
"Date",
// "Experimental"
// "Device-Memory"
"Digest",
"DNT",
// "Experimental"
// "Downlink"
// "ExperimentalNon-StandardDeprecated"
// "DPR"
// "Experimental"
// "Early-Data"
// "Experimental"
// "ECT"
"ETag",
"Expect",
"Expect-CT",
"Expires",
// "Experimental"
// "Feature-Policy"
"Forwarded",
"From",
"Host",
"If-Match",
"If-Modified-Since",
"If-None-Match",
"If-Range",
"If-Unmodified-Since",
"Keep-Alive",
// "Non-StandardDeprecated"
// "Large-Allocation"
"Last-Modified",
"Link",
"Location",
"NEL",
"Origin",
// "Deprecated"
// "Pragma"
"Proxy-Authenticate",
"Proxy-Authorization",
// "Deprecated"
// "Public-Key-Pins"
// "Deprecated"
// "Public-Key-Pins-Report-Only"
"Range",
"Referer",
"Referrer-Policy",
"Retry-After",
// "Experimental"
// "RTT"
"Save-Data",

/** TODO: research of interest */
// "Experimental"
// "Sec-CH-UA"
// "Experimental"
// "Sec-CH-UA-Arch"
// "Experimental"
// "Sec-CH-UA-Bitness"
// "Experimental"
// "Sec-CH-UA-Full-Version"
// "Experimental"
// "Sec-CH-UA-Full-Version-List"
// "Experimental"
// "Sec-CH-UA-Mobile"
// "Experimental"
// "Sec-CH-UA-Model"
// "Experimental"
// "Sec-CH-UA-Platform"
// "Experimental"
// "Sec-CH-UA-Platform-Version"

"Sec-Fetch-Dest",
"Sec-Fetch-Mode",
"Sec-Fetch-Site",
"Sec-Fetch-User",
"Sec-WebSocket-Accept",
"Server",
"Server-Timing",
"Service-Worker-Navigation-Preload",
"Set-Cookie",
"SourceMap",
"Strict-Transport-Security",
"TE",
"Timing-Allow-Origin",
"Tk",
"Trailer",
"Transfer-Encoding",
"Upgrade",
"Upgrade-Insecure-Requests",
"User-Agent",
"Vary",
"Via",
// "ExperimentalNon-StandardDeprecated"
// "Viewport-Width"
"Want-Digest",
// "Deprecated"
// "Warning"
// "ExperimentalDeprecated"
// "Width"
"WWW-Authenticate",
"X-Content-Type-Options",
"X-DNS-Prefetch-Control",
// "Non-Standard"
// "X-Forwarded-For"
// "Non-Standard"
// "X-Forwarded-Host"
// "Non-Standard"
// "X-Forwarded-Proto"
"X-Frame-Options",
"X-XSS-Protection"
};

#endif