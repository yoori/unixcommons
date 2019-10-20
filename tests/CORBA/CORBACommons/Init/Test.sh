#!/bin/sh

data_path="${1}"
shift
export SSL_CERT_FILE="$data_path/CA.pem"
corba_combined="$@ CORBACombined"

base_port=$USER_BASE_PORT
if test -z "$base_port" ; then
  base_port=10000
fi
  
port=`expr $base_port + 4`
secure_port=`expr $base_port + 5`
local_port=`expr $base_port + 2`
local_secure_port=`expr $base_port + 3`
process_control_url="corbaloc::localhost:$port/ProcessControl"
client_secure_params="$data_path/c_key.pem:adserver:$data_path/c_cert.pem:$data_path/CA.pem"
server_secure_params="$data_path/s_key.pem:adserver:$data_path/s_cert.pem:$data_path/CA.pem"
object_url="corbaloc::localhost:$port/TestInt"
secure_object_url="${client_secure_params}@corbaloc:ssliop:localhost:$secure_port/SecureTestInt"


client_insecure=--client="--url=$object_url"
client_secure=--client="--secure-url=$secure_object_url"
client_insecure_secure=--client="--url=$object_url#--secure-url=$secure_object_url"
client_secure_insecure=--client="--secure-url=$secure_object_url#--url=$object_url"

server_insecure=--server="--port=$local_port"
server_secure=--server="--secure-port=$local_secure_port#--secure-params=$server_secure_params"
server_insecure_secure=--server="--port=$local_port#--secure-port=$local_secure_port#--secure-params=$server_secure_params"


echo "Starting CORBAOverloadServer on $object_url ..."

"$@" CORBAOverloadServer --port=$port --secure-port=$secure_port --secure-params="$server_secure_params" &

echo "Test that CORBAOverloadServer alive ...";

"$@" ProbeObj -is-a-mode -retry 1000 -count 120 $object_url

if test $? -ne 0; then
  echo "CORBAOverloadServer isn't alive" >&2
  kill %1
  exit 1
fi

echo "Running $corba_combined ..."

echo "CLIENT ONLY..."
echo "Insecure..."
$corba_combined "$client_insecure"
echo "Secure..."
$corba_combined "$client_secure"
echo "Secure, insecure..."
$corba_combined "$client_insecure_secure"
echo "Insecure, secure..."
$corba_combined "$client_secure_insecure"

echo "SERVER ONLY..."
echo "Insecure..."
$corba_combined "$server_insecure"
echo "Secure..."
$corba_combined "$server_secure"
echo "Insecure, secure..."
$corba_combined "$server_insecure_secure"

echo "CLIENT AND SERVER..."
echo "Insecure and insecure..."
$corba_combined "$client_insecure" "$server_insecure"
echo "Insecure and secure..."
$corba_combined "$client_insecure" "$server_secure"
echo "Secure and insecure..."
$corba_combined "$client_secure" "$server_insecure"
echo "Secure and secure..."
$corba_combined "$client_secure" "$server_secure"

echo "Insecure and insecure, secure..."
$corba_combined "$client_insecure" "$server_insecure_secure"
echo "Secure and insecure, secure..."
$corba_combined "$client_secure" "$server_insecure_secure"

echo "Insecure, secure and insecure..."
$corba_combined "$client_insecure_secure" "$server_insecure"
echo "Secure, insecure and insecure..."
$corba_combined "$client_secure_insecure" "$server_insecure"

echo "Insecure, secure and secure..."
$corba_combined "$client_insecure_secure" "$server_secure"
echo "Secure, insecure and secure..."
$corba_combined "$client_secure_insecure" "$server_secure"

echo "Insecure, secure and insecure, secure..."
$corba_combined "$client_insecure_secure" "$server_insecure_secure"
echo "Secure, insecure and insecure, secure..."
$corba_combined "$client_secure_insecure" "$server_insecure_secure"

echo "SERVER AND CLIENT..."
echo "Insecure and insecure..."
$corba_combined "$server_insecure" "$client_insecure"
echo "Insecure and secure..."
$corba_combined "$server_insecure" "$client_secure"
echo "Secure and insecure..."
$corba_combined "$server_secure" "$client_insecure"
echo "Secure and secure..."
$corba_combined "$server_secure" "$client_secure"

echo "Insecure, secure and insecure..."
$corba_combined "$server_insecure_secure" "$client_insecure"
echo "Insecure, secure and secure..."
$corba_combined "$server_insecure_secure" "$client_secure"

echo "Insecure and insecure, secure..."
$corba_combined "$server_insecure" "$client_insecure_secure"
echo "Insecure and secure, insecure..."
$corba_combined "$server_insecure" "$client_secure_insecure"

echo "Secure and insecure, secure..."
$corba_combined "$server_secure" "$client_insecure_secure"
echo "Secure and secure, insecure..."
$corba_combined "$server_secure" "$client_secure_insecure"

echo "Insecure, secure and insecure, secure..."
$corba_combined "$server_insecure_secure" "$client_insecure_secure"
echo "Insecure, secure and secure, insecure..."
$corba_combined "$server_insecure_secure" "$client_secure_insecure"

"$@" ProbeObj -is-a-mode $process_control_url

if test $? -ne 0; then
  echo "Server not alive" >&2
  exit 1
fi

"$@" ProbeObj -shutdown $process_control_url

if test $? -ne 0; then
  echo "Can't stop CORBAOverloadServer" >&2
  kill %1
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
