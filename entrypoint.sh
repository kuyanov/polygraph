#!/bin/bash

function test_scheduler() {
  ./cmake-build-asan/scheduler/Scheduler &
  sleep 1 && ./cmake-build-asan/test/scheduler/TestScheduler && pkill Scheduler
}

function test_runner() {
  ./cmake-build-asan/runner/Runner &
  sleep 1 && pkill Runner && sleep 1
}

test_scheduler && test_runner
