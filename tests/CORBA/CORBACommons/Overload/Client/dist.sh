t=$1
shift

h=0
for d in $*; do
  h=`expr $h + 1`
  h_h=`echo $d | sed -e 's/=.*//'`
  h_n=`echo $d | sed -e 's/.*=//'`
  eval h_h_$h=\$h_h
  eval h_n_$h=\$h_n
done


echo
date
echo Probing...
build/bin/ProbeObj -is-a-mode -retry 300 -count 3 corbaloc::$t/ProcessControl
if test $? -ne 0; then
  echo "CORBAOverloadServer isn't alive" >&2
  exit 1
fi


echo
date
echo Cleaning...
for h in `seq 1 $h`; do
  eval h_h=\$h_h_$h
  echo $h_h
  ssh $h_h "(killall CORBAOverloadClient; killall flock; sleep 1; killall sleep) >/dev/null 2>/dev/null"
done
sleep 1

echo
date
echo Locking...
for h in `seq 1 $h`; do
  eval h_h=\$h_h_$h
  ssh -f $h_h "(perl -e 'print chr(0)x4096' >&200; flock 200; /bin/sleep 1000) 200>$HOME/lock"
done
sleep 3


echo
date
echo Starting...
for h in `seq 1 $h`; do
  eval h_h=\$h_h_$h
  eval h_n=\$h_n_$h
  echo -n $h_h
  for f in `seq 1 $h_n`; do
    if test `expr $f % 10` = 0 ; then
      echo -n .
    fi
    ssh -f $h_h "LD_LIBRARY_PATH=\$HOME/work/unixcommons/build/lib:\$LD_LIBRARY_PATH \$HOME/work/unixcommons/build/bin/CORBAOverloadClient -ot -u corbaloc::$t/TestInt -lb 1 -rs 0 -t 30 -thr 5 -lf \$HOME/lock >/dev/null"
    sleep .3
  done
  echo
done


echo
echo Press Enter to continue...
read


echo
for f in `seq 0 50`; do
  if test `expr $f % 10` = 0 ; then
    date
    for h in `seq 1 $h`; do
      eval h_h=\$h_h_$h
      echo -n "$h_h "
      ssh $h_h "/usr/sbin/ss -4nto state established dst $t | tail -n +2 | awk '{ a += \$1; b += \$2; c += 1 }; END { print a, b, c }'"
    done
  fi
  if test $f = 0 ; then
    echo Unleashing...
    for h in `seq 1 $h`; do
      eval h_h=\$h_h_$h
      ssh $h_h "killall sleep 2>/dev/null; rm \$HOME/lock" &
    done
  fi
  sleep 1
done


echo
date
echo Stopping...
build/bin/ProbeObj -shutdown corbaloc::$t/ProcessControl
