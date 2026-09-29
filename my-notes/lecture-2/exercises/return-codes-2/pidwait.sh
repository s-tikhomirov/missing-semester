pidwait() {
  kill -0 "$1" 2>/dev/null
  process_still_running=$?
  while [ "$process_still_running" -eq 0 ]; do
    sleep 1
    kill -0 "$1" 2>/dev/null
    process_still_running=$?
  done
}
