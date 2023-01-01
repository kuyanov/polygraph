#!/bin/bash

./$BUILD_DIR/runner/Runner &
sleep 1 && ./$BUILD_DIR/test/runner/TestRunner
STATUS=$?
pkill Runner
exit $STATUS
