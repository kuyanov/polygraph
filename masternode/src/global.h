#pragma once

namespace http {

inline const char *bad_request = "400 Bad Request";
inline const char *not_found = "404 Not Found";
inline const char *request_entity_too_large = "413 Request Entity Too Large";

}  // namespace http

namespace signals {

inline const char *runner_complete = "complete";

}  // namespace signals
