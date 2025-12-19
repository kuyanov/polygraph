#!/usr/bin/env bash

# Scheduler tests
./test_scheduler.sh
STATUS=$?
if [ $STATUS != 0 ]; then
    echo "Scheduler tests failed"
    exit $STATUS
fi

# Runner tests
./test_runner.sh
if [ $STATUS != 0 ]; then
    echo "Runner tests failed"
    exit $STATUS
fi

echo "All tests passed"
