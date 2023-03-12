#pragma once

#define HTTP_BAD_REQUEST "400 Bad Request"
#define HTTP_NOT_FOUND "404 Not Found"
#define HTTP_REQUEST_ENTITY_TOO_LARGE "413 Request Entity Too Large"

#define PARSE_ERROR_PREFIX "Could not parse json: "
#define VALIDATION_ERROR_PREFIX "Validation error: "
#define RUNTIME_ERROR_PREFIX "Runtime error: "
#define API_ERROR_PREFIX "API error: "

#define DUPLICATED_PATH_ERROR "duplicated path"
#define INVALID_CONNECTION_ERROR "invalid connection"
#define UNDEFINED_COMMAND_ERROR "undefined command"
#define NOT_IMPLEMENTED_ERROR "not implemented"
#define ALREADY_RUNNING_ERROR "workflow is already running"

#define RUN_SIGNAL "run"
#define STOP_SIGNAL "stop"

#define RUNNING_STATE "running"
#define COMPLETE_STATE "complete"

#define UUID_REGEX "^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$"
