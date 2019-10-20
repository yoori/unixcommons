my $timeout = shift(@ARGV);
my $pid = 0;
$SIG{TERM} = $SIG{INT} = $SIG{QUIT} =
  sub
  {
    kill(9, -$pid);
    print STDERR "\nKilled by user\n";
    exit 1;
  };
$pid = fork();
if ($pid)
{
  $SIG{ALRM} =
    sub
    {
      kill(9, -$pid);
      print STDERR "\nKilled by timeout\n";
    };
  alarm($timeout * 60);
  wait;
  exit($?);
}
else
{
  setpgrp();
  sleep(1);
  exec(@ARGV);
  exit(-1);
}
