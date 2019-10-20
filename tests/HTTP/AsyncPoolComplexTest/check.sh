APACHE_DIR=$TEST_TMP_DIR/HTTP/AsyncPoolComplexTest/apache
HTTPD="/usr/sbin/httpd -d $APACHE_DIR"
export AsyncPoolComplexTest_server_root="$APACHE_DIR"

mkdir -p $APACHE_DIR/conf $APACHE_DIR/logs

. $TEST_DATA_DIR/conf_gen.sh AsyncPoolComplexTest 1>$APACHE_DIR/conf/httpd.conf 0<$TEST_DATA_DIR/apache/httpd.conf.in

trap 'echo Signal caught' 2

$HTTPD -k start 2>&1

"$@" TestHttpPoolComplex

$HTTPD -k stop 2>&1
