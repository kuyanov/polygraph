#pragma once

#include <string>

#include "json.h"
#include "scheduler.h"

class SchedulerApp {
public:
    static SchedulerApp &Get() {
        static SchedulerApp app;
        return app;
    }

    void Run();

private:
    SchemaValidator workflow_validator_;
    Scheduler scheduler_;
    inline static const std::string listen_host_ = "0.0.0.0";

    SchedulerApp();
};
