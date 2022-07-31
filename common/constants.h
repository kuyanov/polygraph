#pragma once

#include <filesystem>
#include <string>

namespace filesystem {

const std::filesystem::path kSchemaPath = SCHEMA_DIR;
const std::filesystem::path kSandboxPath = SANDBOX_DIR;
const std::filesystem::path kUserPath = USER_DIR;

const std::filesystem::perms kWritePerms = std::filesystem::perms::owner_write |
                                           std::filesystem::perms::group_write |
                                           std::filesystem::perms::others_write;
const std::filesystem::perms kExecPerms = std::filesystem::perms::owner_exec |
                                          std::filesystem::perms::group_exec |
                                          std::filesystem::perms::others_exec;

}  // namespace filesystem

namespace http_response {

const std::string kBadRequest = "400 Bad Request";
const std::string kNotFound = "404 Not Found";
const std::string kRequestEntityTooLarge = "413 Request Entity Too Large";

}  // namespace http_response

namespace errors {

const std::string kParseErrorPrefix = "Could not parse json: ";
const std::string kValidationErrorPrefix = "Invalid document: ";
const std::string kSemanticErrorPrefix = "Semantic error: ";
const std::string kAPIErrorPrefix = "API error: ";
const std::string kRuntimeErrorPrefix = "Runtime error: ";

const std::string kDuplicatedFilename = "duplicated filename";
const std::string kEmptyFilename = "empty filename";
const std::string kInvalidFilename = "invalid filename";
const std::string kInvalidStartBlock = "invalid connection start block";
const std::string kInvalidStartBlockOutput = "invalid connection start block output";
const std::string kInvalidEndBlock = "invalid connection end block";
const std::string kInvalidEndBlockInput = "invalid connection end block input";
const std::string kLoopsNotSupported = "loops are not supported";
const std::string kInvalidMaxRunners = "'max-runners' is invalid";
const std::string kUndefinedCommand = "undefined command";
const std::string kNotImplemented = "not implemented";
const std::string kAlreadyRunning = "graph is already running";

}  // namespace errors

namespace signals {

const std::string kGraphRun = "run";
const std::string kGraphStop = "stop";
const std::string kGraphComplete = "complete";

}  // namespace signals
