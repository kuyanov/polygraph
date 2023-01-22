#pragma once

#include <cstdlib>
#include <string>

inline std::string GetEnvOr(const char *env_name, const std::string &default_value) {
    const char *value = getenv(env_name);
    if (!value) {
        return default_value;
    }
    return value;
}

inline std::string GetConfDir() {
    return GetEnvOr("CONF_DIR", CONF_DIR);
}

inline std::string GetDataDir() {
    return GetEnvOr("DATA_DIR", DATA_DIR);
}

inline std::string GetExecDir() {
    return GetEnvOr("EXEC_DIR", EXEC_DIR);
}

inline std::string GetLogDir() {
    return GetEnvOr("LOG_DIR", LOG_DIR);
}

inline std::string GetRunDir() {
    return GetEnvOr("RUN_DIR", RUN_DIR);
}

inline std::string GetVarDir() {
    return GetEnvOr("VAR_DIR", VAR_DIR);
}
