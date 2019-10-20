#!/usr/bin/perl -w
use strict;
use Cwd;

my $output;
my ($dir, $param) = split(/:/, $ENV{QUERY_STRING} || "");

if ($param > 50)
{
  $output = `/usr/sbin/httpd -d $dir/HTTP/AsyncPoolComplexTest/apache -k stop`;
}
$output = $param;
$output = "<HTML>\n<HEAD></HEAD><BODY>\n" . $output . "\n</BODY></HTML>\n";

my $length = length($output);
print "Content-type: text/html\n";
print("Content-length: $length\n\n");
print($output);
