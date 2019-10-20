#!/bin/sh

data_path="${1}"
export SSL_CERT_FILE="$data_path/CA.pem"
base_port=$USER_BASE_PORT
if test -z "$base_port" ; then
  base_port=10000
fi
  
port=`expr $base_port + 6`
secure_port=`expr $base_port + 7`
  
process_control_url="corbaloc::localhost:$port/ProcessControl"
client_secure_params="$data_path/c_key.pem:adserver:$data_path/c_cert.pem:$data_path/CA.pem"
server_secure_params="$data_path/s_key.pem:adserver:$data_path/s_cert.pem:$data_path/CA.pem"
object_url="corbaloc::localhost:$port/TestCrash"
secure_object_url="${client_secure_params}@corbaloc:ssliop:localhost:$secure_port/SecureTestCrash"


test_crash()
{
echo "Checking $1 connection..."

echo "Starting CORBACrashCallServer..."

CORBACrashCallServer --port=$port --secure-port=$secure_port --secure-params="$server_secure_params" &

echo "Test that CORBACrashCallServer alive...";

ProbeObj -is-a-mode -retry 1000 -count 120 $process_control_url

if test $? -ne 0; then
  echo "CORBACrashCallServer isn't alive" >&2
  kill %1
  exit 1
fi

echo "Running CORBACrashCallClient ..."

CORBACrashCallClient $2

if test $? -ne 0; then
  echo "CORBACrashCallClient return error: $?" >&2
  kill %1
  exit 1
fi

ProbeObj -is-a-mode $process_control_url

if test $? -eq 0; then
  echo "Server is alive" >&2

  ProbeObj -shutdown $process_control_url

  if test $? -ne 0; then
    echo "Can't stop CORBACrashCallServer" >&2
    kill %1
    exit 1
  fi

  sleep 1

  ProbeObj -is-a-mode $process_control_url

  if test $? -eq 0; then
    echo "CORBACrashCallServer is alive after stop" >&2
    kill %1
    exit 1
  fi
fi

}

test_crash insecure --url="$object_url"

test_crash secure --secure-url="$secure_object_url"
