APACHE_DIR=$TEST_TMP_DIR/HTTP/EmptyPoliciesInternalTest/apache
HTTPD="/usr/sbin/httpd -d $APACHE_DIR"

mkdir -p $APACHE_DIR/conf $APACHE_DIR/logs

$TEST_DATA_DIR/conf_gen.sh EmptyPoliciesInternalTest 1>$APACHE_DIR/conf/httpd.conf 0<$TEST_DATA_DIR/apache/httpd_1.conf.in

trap 'echo Signal caught' 2

$HTTPD -k start

#"$@" TestEmptPoliciesInt

$HTTPD -k stop
