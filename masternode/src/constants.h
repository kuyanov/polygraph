#pragma once

#include <string>

namespace http_response {

const std::string kBadRequest = "400 Bad Request";
const std::string kNotFound = "404 Not Found";
const std::string kRequestEntityTooLarge = "413 Request Entity Too Large";

}  // namespace http_response

namespace errors {

const std::string kParseErrorPrefix = "Could not parse json: ";
const std::string kValidationErrorPrefix = "Invalid document: ";
const std::string kSemanticErrorPrefix = "Semantic error: ";

const std::string kDuplicatedFilename = "Duplicated filename";
const std::string kEmptyFilename = "Empty filename";
const std::string kInvalidStartBlock = "Invalid connection start block";
const std::string kInvalidStartBlockOutput = "Invalid connection start block output";
const std::string kInvalidEndBlock = "Invalid connection end block";
const std::string kInvalidEndBlockInput = "Invalid connection end block input";
const std::string kLoopsNotSupported = "Loops are not supported";
const std::string kMaxRunnersAtLeast1 = "max-runners must be >= 1";

}  // namespace errors

namespace signals {

const std::string kGraphRun = "run";
const std::string kGraphStop = "stop";
const std::string kGraphComplete = "complete";

}  // namespace signals
