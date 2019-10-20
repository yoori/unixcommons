#!/bin/sh

data_path="${1}"
shift
base_port=$USER_BASE_PORT
if test -z "$base_port" ; then
  base_port=10000
fi
port=$base_port
secure_port=`expr $base_port + 1`
process_control_url="corbaloc::localhost:$port/ProcessControl"
client_secure_params="$data_path/c_key1.pem:adserver:$data_path/c_cert1.pem:$data_path/CA.pem"
server_secure_params="$data_path/s_key.pem:adserver:$data_path/s_cert.pem:$data_path/CA1.pem"
object_url="corbaloc::localhost:$port/TestInt"
secure_object_url="${client_secure_params}@corbaloc:ssliop:localhost:$secure_port/SecureTestInt"

echo "Starting CORBAOverloadServer on $object_url ..."

"$@" CORBAOverloadServer --host="localhost" --port=$port --secure-port=$secure_port --secure-params="$server_secure_params" &

echo "Test that CORBAOverloadServer alive ...";

"$@" ProbeObj -is-a-mode -retry 1000 -count 120 $object_url

if test $? -ne 0; then
  echo "CORBAOverloadServer isn't alive" >&2
  kill %1
  exit 1
fi

echo "Running CORBAOverloadClient ..."

"$@" CORBAOverloadClient --secure-url="$secure_object_url" --url="$object_url"

if test $? -ne 0; then
  echo "CORBAOverloadClient return error: $?" >&2
  "$@" ProbeObj -shutdown $process_control_url
  exit 1
fi

"$@" ProbeObj -is-a-mode $process_control_url

if test $? -ne 0; then
  echo "Server not alive" >&2
  exit 1
fi

"$@" ProbeObj -control Name Value $process_control_url

"$@" StatsTool "$object_url"

"$@" ProbeObj -shutdown $process_control_url

if test $? -ne 0; then
  echo "Can't stop CORBAOverloadServer" >&2
  exit 1
fi

sleep 1

"$@" ProbeObj -is-a-mode $process_control_url

if test $? -eq 0; then
  echo "CORBAOverloadServer alive after stop" >&2
  kill %1
  exit 1
fi

exit 0
