#!/usr/bin/env bash

# Scheduler test
./test_scheduler.sh
STATUS=$?
if [ $STATUS != 0 ]; then
    exit $STATUS
fi

# Runner test
./test_runner.sh
if [ $STATUS != 0 ]; then
    exit $STATUS
fi

echo "All tests passed"
