#!/bin/bash

STATUS=0

function test_runner() {
  ./cmake-build-asan/runner/Runner 2>> stderr.log &
  ./cmake-build-asan/test/runner/TestRunner
  STATUS=$(($STATUS + $?))
  pkill Runner
}

function test_scheduler() {
  ./cmake-build-asan/scheduler/Scheduler 2>> stderr.log &
  sleep 1 && ./cmake-build-asan/test/scheduler/TestScheduler
  STATUS=$(($STATUS + $?))
  pkill Scheduler
}

test_runner
test_scheduler
cat stderr.log
exit $STATUS
