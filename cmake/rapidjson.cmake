include(FetchContent)

FetchContent_Declare(
    rapidjson
    GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
    GIT_SUBMODULES ""
    SOURCE_SUBDIR include
)

FetchContent_MakeAvailable(rapidjson)
