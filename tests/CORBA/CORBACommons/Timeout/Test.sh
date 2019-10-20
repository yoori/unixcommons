#!/bin/sh

export ORB_TIMEOUT=4

if test -z "$USER_BASE_PORT" ; then
  USER_BASE_PORT=10000
fi
export USER_BASE_PORT=`expr $USER_BASE_PORT + 13`

exec "$@"
