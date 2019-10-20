#!/bin/sh

if [ "$#" -lt  1 ]; then

  echo "Invalid args."
  echo "USAGE: conf_gen.sh prefix 0<template_file 1>result_file"

else

std_prefix="common"
std_prefix_len=`expr ${#std_prefix} + 1`

prefix=$1
prefix_len=`expr ${#prefix} + 1`

script="$TEST_TMP_DIR/conf_gen_sed_script.$$"
echo >"$script"

#Individual params
for param in `set | grep "^${prefix}_"`
do
  param_name=`expr match "$param": '\(.*\)='`
  clean_param_name=${param_name:prefix_len}
  echo -n "s/@$clean_param_name@/" >>"$script"
  echo -n `eval "echo -n \\$${param_name} | sed -e 's/\\//\\\\\&/g'"` >>"$script"
  echo "/g" >>"$script"
  shift
done

#Common params
for param in `set | grep "^${std_prefix}_"`
do
  param_name=`expr match "$param": '\(.*\)='`
  clean_param_name=${param_name:std_prefix_len}
  echo -n "s/@$clean_param_name@/" >>"$script"
  echo -n `eval "echo -n \\$${param_name} | sed -e 's/\\//\\\\\&/g'"` >>"$script"
  echo "/g" >>"$script"
  shift
done

sed -f "$script"
rm -f "$script"

fi;
