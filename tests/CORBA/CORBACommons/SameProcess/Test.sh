#!/bin/sh

data_path="${1}"
shift
base_port=$USER_BASE_PORT
if test -z "$base_port" ; then
  base_port=10000
fi
port=`expr $base_port + 13`
secure_port=`expr $base_port + 14`
client_secure_params="$data_path/c_key1.pem:adserver:$data_path/c_cert1.pem:$data_path/CA.pem"
server_secure_params="$data_path/s_key.pem:adserver:$data_path/s_cert.pem:$data_path/CA1.pem"
object_url="corbaloc::localhost:$port/TestInt"
secure_object_url="${client_secure_params}@corbaloc:ssliop:localhost:$secure_port/TestInt"

"$@" CORBASameProcess --server="--port=$port#--secure-port=$secure_port#--secure-params=$server_secure_params" --client="--secure-url=$secure_object_url#--url=$object_url"
