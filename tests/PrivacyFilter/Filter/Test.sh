pwd=`pwd`
export LD_LIBRARY_PATH=`echo $LD_LIBRARY_PATH | sed -e "s:^[^/]:$pwd/&:"`
#echo $LD_LIBRARY_PATH

dir=$1
shift
testfilter="$@ `which TestFilter`"

unset loglevel_control
cd /
if [ -f loglevel.control ] ; then
  echo "Invalid environment for the test" >&2
fi

res=`$testfilter A B`
if [ "X$res" != "XB" ] ; then
  echo "Invalid behaviour for absence of key" >&2
fi

cd $dir
res=`$testfilter A B`
if [ "X$res" != "XA" ] ; then
  echo "Invalid behaviour for presence of key in the current directory" >&2
fi

cd /
export loglevel_control=$dir/loglevel.control
res=`$testfilter A B`
if [ "X$res" != "XA" ] ; then
  echo "Invalid behaviour for presence of key in the enviroment var" >&2
fi

cd $dir
res=`$testfilter A B`
if [ "X$res" != "XA" ] ; then
  echo "Invalid behaviour for presence of key everywhere" >&2
fi

echo "Tests completed"
