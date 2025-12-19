#pragma once

#define HTTP_BAD_REQUEST "400 Bad Request"
#define HTTP_NOT_FOUND "404 Not Found"
#define HTTP_REQUEST_ENTITY_TOO_LARGE "413 Request Entity Too Large"

#define SUBMIT_ACCEPTED "accepted"
#define SUBMIT_PARSE_ERROR "parse error"
#define SUBMIT_VALIDATION_ERROR "validation error"

#define DUPLICATED_PATH_ERROR "duplicated path"
#define INVALID_CONNECTION_ERROR "invalid connection"
#define UNDEFINED_COMMAND_ERROR "undefined command"
#define NOT_IMPLEMENTED_ERROR "not implemented"
#define ALREADY_RUNNING_ERROR "workflow is already running"

#define BLOCK_SIGNAL "block"
#define ERROR_SIGNAL "error"
#define RUN_SIGNAL "run"
#define STOP_SIGNAL "stop"
#define WORKFLOW_SIGNAL "workflow"

#define RUNNING_STATE "running"
#define FINISHED_STATE "finished"
