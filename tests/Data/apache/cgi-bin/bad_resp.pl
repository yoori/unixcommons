#!/usr/bin/perl -w
use strict;

print "Content-type: text/html\n";
my $output;
my $param=$ENV{QUERY_STRING};

if ($param > 50)
{
  print "Content-type: $param\n";
  print "Keep-Alive:::: want-more\n";
  print ":Set-Cookie: #####\n";
  print "Content-length: pclose\n";
}
else
{
  $output =
    "<HTML>\n<HEAD></HEAD><BODY>\n" .
    $ENV{QUERY_STRING} .
    "\n</BODY></HTML>\n";
}

my $length = length($output);
print("Content-length: $length\n\n");
print($output);
