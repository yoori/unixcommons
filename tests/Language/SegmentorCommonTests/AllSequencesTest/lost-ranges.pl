#! /usr/bin/perl
# -*- cperl -*-
#
use warnings;
use strict;

=head1 NAME

lost-ranges.pl - determine missing UTF-8 ranges in AllSequencesTest output.

=head1 SYNOPSIS

  lost-ranges.pl < AllSequencesTest.out

=cut


sub hex2code {
    my ($hex) = @_;

    my $utf8 = pack("C*", map { hex } split ' ', $hex);

    utf8::decode($utf8);

    return ord($utf8);
}


sub code2hex {
    my ($code) = @_;

    my $unicode;
    {
        no warnings 'utf8';     # Suppress warning about illegal 0xffff.
        $unicode = chr($code);
    }

    utf8::encode($unicode);

    return join ' ', map { sprintf "%02x", $_ } unpack("C*", $unicode);
}


print "Missing UTF-8 symbols:\n";

my $start;
my $end = -1;

sub print_range {
    if (defined $start) {
        if ($start != $end - 1) {
            print code2hex($start), " -- ", code2hex($end - 1), "\n";
        } else {
            print code2hex($start), "\n";
        }
    }
}

while (<STDIN>) {
    next unless /((?: [[:xdigit:]]{2})+) => ''$/;

    my $code = hex2code($1);

    if ($code > $end) {
        print_range;
        $start = $code;
        $end = $code + 1;
    } else {
        ++$end;
    }
}

print_range;
