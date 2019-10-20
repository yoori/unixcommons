#!/bin/sh

APACHE=/usr/sbin/httpd

case "$1" in
  ("start")
    flock -o "$common_lock" "$0" safe_start
    ;;
  ("stop")
    flock -o "$common_lock" "$0" safe_stop
    ;;
  ("safe_start")
    num=`cat $common_lock`
    if [ -z "$num" ]
    then
      num=0
    fi
    if [ "$num" -eq "0" ]
    then
      for i in 1 2 3 4
      do
        ROOT="$TEST_TMP_DIR/apache_$i"
        eval export "apache_${i}_server_root"=\"$ROOT\"
        mkdir -p "$ROOT/conf" "$ROOT/logs" "$ROOT/run"
        "$TEST_DATA_DIR/conf_gen.sh" "apache_$i" 0<"$TEST_DATA_DIR/apache/httpd.conf.in" 1>"$ROOT/conf/httpd.conf"
        $APACHE -d "$ROOT" -k start
        sleep 1
      done
    fi
    num=`/usr/bin/expr $num + 1`
    echo $num >$common_lock
    ;;
  ("safe_stop")
    num=`cat $common_lock`
    if [ -z "$num" ]
    then
      num="-1"
    else
      num=`/usr/bin/expr $num - 1`
    fi
    if [ "$num" -eq "0" ]
    then
      for i in 1 2 3 4
      do
        $APACHE -d "$TEST_TMP_DIR/apache_$i" -k stop
        sleep 1
      done
    fi
    if [ "$num" -ge "0" ]
    then
      echo $num >$common_lock
    fi
    ;;
esac
