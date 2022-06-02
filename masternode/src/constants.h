#pragma once

#include <filesystem>
#include <string>

namespace filesystem {

inline const std::filesystem::path kSchemaPath = SCHEMA_DIR;
inline const std::filesystem::path kSandboxPath = SANDBOX_DIR;

}  // namespace filesystem

namespace http_response {

inline const std::string kBadRequest = "400 Bad Request";
inline const std::string kNotFound = "404 Not Found";
inline const std::string kRequestEntityTooLarge = "413 Request Entity Too Large";

}  // namespace http_response

namespace errors {

inline const std::string kParseErrorPrefix = "Could not parse json: ";
inline const std::string kValidationErrorPrefix = "Invalid document: ";
inline const std::string kSemanticErrorPrefix = "Semantic error: ";

inline const std::string kDuplicatedFilename = "Duplicated filename";
inline const std::string kEmptyFilename = "Empty filename";
inline const std::string kInvalidStartBlock = "Invalid connection start block";
inline const std::string kInvalidStartBlockOutput = "Invalid connection start block output";
inline const std::string kInvalidEndBlock = "Invalid connection end block";
inline const std::string kInvalidEndBlockInput = "Invalid connection end block input";
inline const std::string kLoopsNotSupported = "Loops are not supported";
inline const std::string kMaxRunnersAtLeast1 = "max-runners must be >= 1";

}  // namespace errors

namespace signals {

inline const std::string kGraphRun = "run";
inline const std::string kGraphStop = "stop";
inline const std::string kGraphComplete = "complete";

}  // namespace signals
