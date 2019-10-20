#!/bin/sh

base_port=$USER_BASE_PORT
if test -z "$base_port" ; then
  base_port=10000
fi
port=`expr $base_port + 40`
if test -z "$TEST_TMP_DIR" ; then
  TEST_TMP_DIR=`pwd`
fi
if test -z "$TEST_TOP_SRC_DIR" ; then
  TEST_TOP_SRC_DIR=`pwd`
fi
agentx=$TEST_TMP_DIR/agentx
pid=$TEST_TMP_DIR/snmpd.pid
/usr/sbin/snmpd -x $agentx -p $pid :$port
( sleep 8 ; snmpwalk -v2c -c public -M +$TEST_TOP_SRC_DIR/share/snmp/mibs:$TEST_TOP_SRC_DIR/tests/SNMP/SNMPSimple localhost:$port SNMPSimple-MIB::SNMPSimple ) &
sleep 1
( sleep 10 ; echo ) | "$@" TestSNMPSimple NaMe $agentx
kill `cat $pid`
rm $agentx
