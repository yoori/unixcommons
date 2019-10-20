#!/bin/sh

data_path="${1}"
base_port=$USER_BASE_PORT
if test -z "$base_port" ; then
  base_port=10000
fi

port=`expr $base_port + 8`

process_control_url="corbaloc::localhost:$port/ProcessControl"
pool_obj_url="corbaloc::localhost:$port/PoolObj"
objects_count=5

GREEN='\033[0;32m'
RED='\033[0;31m'
NORMAL='\033[0m'

echo -e ${GREEN}"Checking insecure connection..."
echo -e "Starting CORBAObjectPoolServer..."${NORMAL}

"$@" CORBAObjectPoolServer --port="$port" --objects_amount="$objects_count" &

echo -e ${GREEN}"Test that CORBAObjectPoolServer alive..."${NORMAL};

"$@" ProbeObj -is-a-mode -retry 1000 -count 120 $process_control_url


if test $? -ne 0; then
  echo -e ${RED}"CORBAObjectPoolServer isn't alive"${NORMAL} >&2
  kill %1
  exit 1
fi

echo -e ${GREEN}"Running CORBAObjectPoolClient ..."${NORMAL}

"$@" CORBAObjectPoolTestClient --purl="$pool_obj_url"

if test $? -ne 0; then
  echo -e ${RED}"CORBAObjectPoolClient return error: $?"${NORMAL} >&2
  kill %1
  exit 1
fi

"$@" ProbeObj -is-a-mode $process_control_url

if test $? -eq 0; then

  "$@" ProbeObj -shutdown $process_control_url

  if test $? -ne 0; then
    echo -e ${RED}"Can't stop CORBAObjectPoolServer"${NORMAL} >&2
    kill %1
    exit 1
  fi

  sleep 1

  "$@" ProbeObj -is-a-mode $process_control_url

  if test $? -eq 0; then
    echo -e ${RED}"CORBAObjectPoolServer is alive after stop"${NORMAL} >&2
    kill %1
    exit 1
  fi
fi
