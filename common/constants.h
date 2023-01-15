#pragma once

#include <cstdlib>
#include <string>

namespace http_response {

const std::string kBadRequest = "400 Bad Request";
const std::string kNotFound = "404 Not Found";
const std::string kRequestEntityTooLarge = "413 Request Entity Too Large";

}  // namespace http_response

namespace errors {

const std::string kParseErrorPrefix = "Could not parse json: ";
const std::string kValidationErrorPrefix = "Validation error: ";
const std::string kRuntimeErrorPrefix = "Runtime error: ";
const std::string kAPIErrorPrefix = "API error: ";

const std::string kDuplicatedLocation = "duplicated location";
const std::string kInvalidConnection = "invalid connection";
const std::string kLoopsNotSupported = "loops are not supported";
const std::string kUndefinedCommand = "undefined command";
const std::string kNotImplemented = "not implemented";
const std::string kAlreadyRunning = "workflow is already running";

}  // namespace errors

namespace signals {

const std::string kRun = "run";
const std::string kStop = "stop";

}  // namespace signals

namespace states {

const std::string kRunning = "running";
const std::string kComplete = "complete";

}  // namespace states

namespace paths {

const std::string kConfDir = getenv("CONF_DIR") ? getenv("CONF_DIR") : CONF_DIR;
const std::string kDataDir = getenv("DATA_DIR") ? getenv("DATA_DIR") : DATA_DIR;
const std::string kExecDir = getenv("EXEC_DIR") ? getenv("EXEC_DIR") : EXEC_DIR;
const std::string kLogDir = getenv("LOG_DIR") ? getenv("LOG_DIR") : LOG_DIR;
const std::string kVarDir = getenv("VAR_DIR") ? getenv("VAR_DIR") : VAR_DIR;

}  // namespace paths
