#!/bin/bash

./cmake-build-asan/runner/Runner 2>> stderr.log &
./cmake-build-asan/test/runner/TestRunner
STATUS=$?
pkill Runner
cat stderr.log
exit $STATUS
