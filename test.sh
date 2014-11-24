#!/bin/bash

assertContains()
{
  echo -n "$1"
  if [[ "$3" == *$2* ]]; then
    echo "Pass"
  else
    echo "Fail"
    echo
    echo "  Expected: \"${2}\""
    echo "  Received: \"${3}\""
    echo
    return 1
  fi
  return 0
}

assertEqual()
{
  echo -n "$1"
  if [ "$2" == "$3" ]; then
    echo "Pass"
  else
    echo "Fail"
    echo
    echo "  Expected: \"${2}\""
    echo "  Received: \"${3}\""
    echo
    return 1
  fi
  return 0
}

if [ "$TRAVIS" == "true" ]; then
  echo "Testing pg-statsd"
  psql -U postgres -f test.sql 
else
  su postgres -c "psql -c 'CREATE EXTENSION statsd'"
  echo "Testing pg-statsd"
  su postgres -c "psql -q -o /dev/null -f test.sql"
  su postgres -c "psql -c 'DROP EXTENSION statsd'"
  su postgres -c "psql -c 'DROP SCHEMA statsd'"
fi
echo "Getting results"

RESULT=`echo counters | nc localhost 8126 | grep -v 'END'`
assertContains "Counters: " "'test-counter-1': 7" "$RESULT"
CONTAINERS=$?

RESULT=`echo gauges | nc localhost 8126 | grep -v 'END'`
assertContains "Gauges: " "'test-gauge': 98" "$RESULT"
GAUGES=$?

RESULT=`echo timers | nc localhost 8126 | grep -v 'END'`
assertEqual "Timers: " "{ 'test-timing1': [ 70, 50 ] }" "$RESULT"
TIMERS=$?

echo "Clearing statsd"
echo "delcounters" | nc localhost 8126 | grep -G "/[END]|\n/"
echo "delgauges" | nc localhost 8126 | grep -G "/[END]|\n/"
echo "deltimers" | nc localhost 8126 | grep -G "/[END]|\n/"

exit $(($CONTAINERS+$GAUGES+$TIMERS))
