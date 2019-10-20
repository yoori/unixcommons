rm -rf result_keys
mkdir result_keys

cp openssl.cnf result_keys/
cd result_keys

echo "GENERATE CA:"

../CA.sh -newca -root

echo "GENERATE Client key:"

../CA.sh -newreq
../CA.sh -sign

mv newpriv.pem c_key.pem
grep -A 1000 "^-----BEGIN " newcert.pem >c_cert.pem
rm newreq.pem newcert.pem

echo "GENERATE Server key:"

../CA.sh -newreq
../CA.sh -sign

mv newpriv.pem s_key.pem
grep -A 1000 "^-----BEGIN " newcert.pem >s_cert.pem
rm newreq.pem newcert.pem

cat topCA/cacert.pem >CA.pem
