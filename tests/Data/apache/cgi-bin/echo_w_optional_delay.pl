#!/usr/bin/perl -w
use strict;

my $cookie = $ENV{'HTTP_COOKIE'} || "";
$cookie = $1 if $cookie =~ /\=([^ ;,\n]*)/;
$cookie = 0 if $cookie =~ /[^0-9]/;
++$cookie;
$cookie = "COOKIE=$cookie;";

my $output;
my $length;
if ($ENV{'REQUEST_METHOD'} eq "POST")
{
  $length = $ENV{'CONTENT_LENGTH'};
  $output = "Invalid request" if sysread(STDIN, $output, $length) != $length;
}
else
{
  $output = $ENV{'QUERY_STRING'};
}

my $delay_value;
if ($output =~ /delay=?(\d*)/o)
{
  if ($1 > 1)
  {
    sleep $1;
  }
  else
  {
    sleep 1;
  }
}

$output = "<HTML>
<HEAD><TITLE>$cookie</TITLE></HEAD>
<BODY>
$output
</BODY>
</HTML>";

$length = length($output);
print("Content-type: text/html\r
Content-length: $length\r
Set-Cookie: $cookie\r
\r
$output");
