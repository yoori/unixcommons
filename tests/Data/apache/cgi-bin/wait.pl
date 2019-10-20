#!/usr/bin/perl -W
use strict;

my $wait = $ENV{'QUERY_STRING'} || "";
$wait =~ s/[^0-9]//;
$wait = 10 unless length($wait);
sleep($wait);

print("Content-type: text/html\r
Content-length: 0\r
\r
");
