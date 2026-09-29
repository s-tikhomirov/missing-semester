#!/usr/bin/env bash

LAST_EXIT_STATUS=0
NUM_RUNS=0

while [ "$LAST_EXIT_STATUS" -eq 0 ]
do
  #echo "Running randomly failing script"
  NUM_RUNS=$((NUM_RUNS+1))
  ./fails_randomly.sh >out.log 2>err.log
  LAST_EXIT_STATUS=$?
done

cat out.log err.log
echo "Number of runs: $NUM_RUNS"