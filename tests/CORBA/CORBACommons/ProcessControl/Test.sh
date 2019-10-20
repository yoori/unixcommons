#!/bin/sh

host="localhost"

base_port=$USER_BASE_PORT

if test -z "$base_port" ; then
  base_port=10000
fi

port=`expr $base_port + 10`

endpoint=iiop://$host:$port
object_url=corbaloc:iiop:$host:$port/ProcessControl

echo "Status check..."
"$@" ProbeObj -retry 2000 -count 3 -status ready $object_url
"$@" ProbeObj -is-a-mode $object_url

if test $? -eq 0; then
  echo "Object $object_url IS running"
else
  echo "Object $object_url IS NOT running"
  echo "Starting TestProcessControl on $object_url ..."
  "$@" TestProcessControl -ORBEndpoint $endpoint &

  sleep 3
  "$@" ProbeObj -is-a-mode -retry 3000 -message FAILURE_DESC $object_url
  echo "Status check..."
  "$@" ProbeObj -retry 2000 -status ready $object_url
fi

echo "
Sleeping for 5 second ...
"
sleep 5

echo "Shutting down object ..."
"$@" ProbeObj -shutdown -wait-for-completion $object_url
"$@" ProbeObj -is-a-mode $object_url

if test $? -eq 0; then
  echo "Object IS NOT terminated"

  echo "Sleeping for 5 second ..."
  sleep 5

  echo "Reprobing the object ..."
  "$@" ProbeObj -is-a-mode $object_url

  if test $? -eq 0; then
    echo "Object still NOT terminated, giving up"
    kill %1
  else
    echo "Object IS terminated at last"
  fi

else
  echo "Object IS terminated"
fi
