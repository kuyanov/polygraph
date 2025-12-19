#include "client.h"
#include "helpers.h"
#include "run.h"

void Run(const RunOptions &options) {
    RequireUp();
    Client::Get(options.workflow_file).Run();
}
