#!/bin/bash

./$BUILD_DIR/scheduler/Scheduler &
sleep 1 && ./$BUILD_DIR/test/scheduler/TestScheduler
STATUS=$?
pkill Scheduler
exit $STATUS
